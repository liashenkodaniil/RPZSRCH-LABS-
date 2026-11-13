// Оркестратор місії прибирання номера на основі дерева поведінки (BehaviorTree.CPP v4).
// Логіка місії задається XML-деревом (trees/hotel_cleaning.xml), а кожен листок
// ExecuteTask лише відправляє ціль виконавцю через Action /execute_task
// і НЕ блокує обхід дерева (StatefulActionNode повертає RUNNING).

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "ament_index_cpp/get_package_share_directory.hpp"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "lab3_interfaces/action/execute_task.hpp"

using ExecuteTask = lab3_interfaces::action::ExecuteTask;
using GoalHandle = rclcpp_action::ClientGoalHandle<ExecuteTask>;
using namespace std::chrono_literals;

// Листок дерева: асинхронний виклик навички виконавця
class ExecuteTaskNode : public BT::StatefulActionNode
{
public:
  ExecuteTaskNode(
    const std::string & name, const BT::NodeConfig & config, rclcpp::Node::SharedPtr node)
  : BT::StatefulActionNode(name, config), node_(std::move(node))
  {
    client_ = rclcpp_action::create_client<ExecuteTask>(node_, "execute_task");
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("task", "Назва навички виконавця"),
      BT::InputPort<std::string>("argument", "", "Аргумент навички"),
      BT::OutputPort<std::string>("result_data", "Дані результату (у Blackboard)"),
      BT::OutputPort<std::string>("message", "Текстове повідомлення результату")};
  }

  // Перший тік: відправляємо ціль і одразу повертаємо RUNNING
  BT::NodeStatus onStart() override
  {
    auto task = getInput<std::string>("task");
    if (!task) {
      throw BT::RuntimeError("ExecuteTask: відсутній порт task: ", task.error());
    }
    if (!client_->wait_for_action_server(2s)) {
      RCLCPP_ERROR(node_->get_logger(), "[BT] Сервер /execute_task недоступний");
      return BT::NodeStatus::FAILURE;
    }

    ExecuteTask::Goal goal;
    goal.task = task.value();
    goal.argument = getInput<std::string>("argument").value_or("");

    done_ = false;
    rejected_ = false;
    goal_handle_.reset();

    rclcpp_action::Client<ExecuteTask>::SendGoalOptions options;
    options.goal_response_callback = [this](const GoalHandle::SharedPtr & handle) {
        if (!handle) {
          rejected_ = true;
        }
        goal_handle_ = handle;
      };
    options.feedback_callback = [this](
      GoalHandle::SharedPtr, const std::shared_ptr<const ExecuteTask::Feedback> fb) {
        RCLCPP_DEBUG(node_->get_logger(), "[BT] feedback: %s", fb->status.c_str());
      };
    options.result_callback = [this](const GoalHandle::WrappedResult & result) {
        result_ = result;
        done_ = true;
      };

    RCLCPP_INFO(
      node_->get_logger(), "[BT] %s -> ціль '%s' (%s)", name().c_str(),
      goal.task.c_str(), goal.argument.c_str());
    client_->async_send_goal(goal, options);
    return BT::NodeStatus::RUNNING;
  }

  // Наступні тіки: лише перевіряємо, чи прийшов результат (без блокування)
  BT::NodeStatus onRunning() override
  {
    if (rejected_) {
      RCLCPP_WARN(node_->get_logger(), "[BT] %s: ціль відхилено", name().c_str());
      return BT::NodeStatus::FAILURE;
    }
    if (!done_) {
      return BT::NodeStatus::RUNNING;
    }

    const bool ok = result_.code == rclcpp_action::ResultCode::SUCCEEDED &&
      result_.result && result_.result->success;
    if (result_.result) {
      setOutput("result_data", result_.result->data);
      setOutput("message", result_.result->message);
      RCLCPP_INFO(
        node_->get_logger(), "[BT] %s -> %s: %s", name().c_str(),
        ok ? "SUCCESS" : "FAILURE", result_.result->message.c_str());
    }
    return ok ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
  }

  // Дерево перервало вузол (наприклад, реактивна умова): скасовуємо ціль
  void onHalted() override
  {
    if (goal_handle_ && !done_) {
      client_->async_cancel_goal(goal_handle_);
      RCLCPP_INFO(node_->get_logger(), "[BT] %s: ціль скасовано", name().c_str());
    }
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp_action::Client<ExecuteTask>::SharedPtr client_;
  GoalHandle::SharedPtr goal_handle_;
  GoalHandle::WrappedResult result_;
  bool done_{false};
  bool rejected_{false};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("bt_orchestrator");

  const std::string default_tree =
    ament_index_cpp::get_package_share_directory("lab3_hotel") + "/trees/hotel_cleaning.xml";
  const auto tree_file = node->declare_parameter<std::string>("tree_file", default_tree);
  const auto season = node->declare_parameter<std::string>("season", "winter");
  const auto tick_period = node->declare_parameter<int>("tick_period_ms", 100);

  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<ExecuteTaskNode>("ExecuteTask", node);

  // Blackboard: спільна пам'ять листків. Сезон задається ззовні параметром вузла.
  auto blackboard = BT::Blackboard::create();
  blackboard->set<std::string>("season", season);

  auto tree = factory.createTreeFromFile(tree_file, blackboard);
  BT::StdCoutLogger cout_logger(tree);  // друк усіх переходів станів вузлів
  BT::printTreeRecursively(tree.rootNode());

  // Місію запускаємо лише тоді, коли виконавець навичок доступний
  auto probe = rclcpp_action::create_client<ExecuteTask>(node, "execute_task");
  while (rclcpp::ok() && !probe->wait_for_action_server(1s)) {
    RCLCPP_INFO(node->get_logger(), "[BT] Очікування сервера /execute_task...");
  }

  RCLCPP_INFO(node->get_logger(), "[BT] Старт місії, сезон: %s", season.c_str());
  const auto start = node->now();

  // Цикл тіків: tickOnce() не блокується, між тіками обробляємо callbacks ROS
  BT::NodeStatus status = BT::NodeStatus::RUNNING;
  while (rclcpp::ok() && status == BT::NodeStatus::RUNNING) {
    status = tree.tickOnce();
    rclcpp::spin_some(node);
    tree.sleep(std::chrono::milliseconds(tick_period));
  }

  RCLCPP_INFO(
    node->get_logger(), "[BT] Місію завершено зі статусом %s за %.1f с",
    BT::toStr(status, false).c_str(), (node->now() - start).seconds());

  rclcpp::shutdown();
  return status == BT::NodeStatus::SUCCESS ? 0 : 1;
}
