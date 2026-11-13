// Вузол-виконавець навичок робота прибирання номерів готелю.
// Він НЕ приймає рішень про порядок дій (це робота оркестратора: BT або FSM),
// а лише виконує окремі навички через стандартний інтерфейс Action
// lab3_interfaces/action/ExecuteTask. Стан "номера" симулюється параметрами.

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "lab3_interfaces/action/execute_task.hpp"

using ExecuteTask = lab3_interfaces::action::ExecuteTask;
using GoalHandle = rclcpp_action::ServerGoalHandle<ExecuteTask>;

class RoomServiceExecutor : public rclcpp::Node
{
public:
  RoomServiceExecutor()
  : Node("room_service_executor")
  {
    // Параметри симуляції стану номера
    declare_parameter<std::vector<std::string>>("missing_items", {"pillow"});
    declare_parameter<bool>("stain_present", true);
    declare_parameter<int>("cleaning_failures", 2);   // скільки спроб чищення будуть невдалими
    declare_parameter<double>("step_duration", 0.3);  // тривалість одного кроку навички, с
    declare_parameter<int>("steps", 4);               // кількість кроків (feedback) на навичку

    using namespace std::placeholders;
    server_ = rclcpp_action::create_server<ExecuteTask>(
      this, "execute_task",
      std::bind(&RoomServiceExecutor::handle_goal, this, _1, _2),
      std::bind(&RoomServiceExecutor::handle_cancel, this, _1),
      std::bind(&RoomServiceExecutor::handle_accepted, this, _1));

    RCLCPP_INFO(get_logger(), "Виконавець навичок запущено (action: /execute_task)");
  }

private:
  static bool known_task(const std::string & task)
  {
    static const std::vector<std::string> tasks{
      "check_bedding", "restock_bedding", "clean_stain",
      "replace_linen", "replace_blanket", "report_issue"};
    return std::find(tasks.begin(), tasks.end(), task) != tasks.end();
  }

  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID &, std::shared_ptr<const ExecuteTask::Goal> goal)
  {
    if (!known_task(goal->task)) {
      RCLCPP_WARN(get_logger(), "Невідома навичка '%s', ціль відхилено", goal->task.c_str());
      return rclcpp_action::GoalResponse::REJECT;
    }
    RCLCPP_INFO(
      get_logger(), "Отримано ціль: task='%s' argument='%s'",
      goal->task.c_str(), goal->argument.c_str());
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandle>)
  {
    RCLCPP_INFO(get_logger(), "Запит на скасування навички");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle)
  {
    // Виконання в окремому потоці, щоб не блокувати executor
    std::thread{
      std::bind(&RoomServiceExecutor::execute, this, std::placeholders::_1),
      goal_handle}.detach();
  }

  void execute(const std::shared_ptr<GoalHandle> goal_handle)
  {
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<ExecuteTask::Feedback>();
    auto result = std::make_shared<ExecuteTask::Result>();

    const int steps = std::max(1, static_cast<int>(get_parameter("steps").as_int()));
    const double step_duration = get_parameter("step_duration").as_double();

    // Імітація тривалої фізичної дії з періодичним feedback
    for (int i = 1; i <= steps; ++i) {
      if (goal_handle->is_canceling()) {
        result->success = false;
        result->message = "Скасовано";
        goal_handle->canceled(result);
        RCLCPP_INFO(get_logger(), "Навичку '%s' скасовано", goal->task.c_str());
        return;
      }
      std::this_thread::sleep_for(std::chrono::duration<double>(step_duration));
      feedback->progress = static_cast<float>(i) / static_cast<float>(steps);
      feedback->status = goal->task + ": крок " + std::to_string(i) + "/" + std::to_string(steps);
      goal_handle->publish_feedback(feedback);
    }

    perform(goal->task, goal->argument, *result);

    if (result->success) {
      RCLCPP_INFO(get_logger(), "[SUCCESS] %s: %s", goal->task.c_str(), result->message.c_str());
      goal_handle->succeed(result);
    } else {
      RCLCPP_WARN(get_logger(), "[FAILURE] %s: %s", goal->task.c_str(), result->message.c_str());
      goal_handle->abort(result);
    }
  }

  // Логіка "світу": що відбувається в номері при виконанні навички
  void perform(const std::string & task, const std::string & argument, ExecuteTask::Result & r)
  {
    std::lock_guard<std::mutex> lock(world_mutex_);

    if (task == "check_bedding") {
      // Початок обслуговування нового номера: скидаємо лічильник спроб чищення
      clean_attempts_ = 0;
      std::string list;
      const auto missing = get_parameter("missing_items").as_string_array();
      for (const auto & item : missing) {
        if (!item.empty()) {
          list += (list.empty() ? "" : ",") + item;
        }
      }
      r.data = list;
      r.success = list.empty();
      r.message = r.success ? "Подушки та простирадла на місці" : "Бракує: " + list;
    } else if (task == "restock_bedding") {
      // Поповнюємо відсутні предмети (стан номера змінюється)
      set_parameter(rclcpp::Parameter("missing_items", std::vector<std::string>{""}));
      r.success = true;
      r.data = argument;
      r.message = "Доставлено з візка: " + (argument.empty() ? std::string("-") : argument);
    } else if (task == "clean_stain") {
      if (!get_parameter("stain_present").as_bool()) {
        r.success = true;
        r.message = "Плям немає";
      } else {
        ++clean_attempts_;
        const int failures = static_cast<int>(get_parameter("cleaning_failures").as_int());
        r.data = std::to_string(clean_attempts_);
        if (clean_attempts_ <= failures) {
          r.success = false;
          r.message = "Пляма залишилась (спроба " + std::to_string(clean_attempts_) + ")";
        } else {
          set_parameter(rclcpp::Parameter("stain_present", false));
          r.success = true;
          r.message = "Пляму видалено (спроба " + std::to_string(clean_attempts_) + ")";
        }
      }
    } else if (task == "replace_linen") {
      r.success = true;
      r.message = "Постільну білизну замінено";
    } else if (task == "replace_blanket") {
      static const std::map<std::string, std::string> blankets{
        {"warm_duvet", "тепла пухова ковдра (зима)"},
        {"light_blanket", "легке покривало (літо)"},
        {"all_season_duvet", "демісезонна ковдра (весна/осінь)"}};
      auto it = blankets.find(argument);
      r.success = it != blankets.end();
      r.message = r.success ? "Встановлено: " + it->second : "Невідомий тип ковдри: " + argument;
    } else if (task == "report_issue") {
      r.success = true;
      r.message = "Персонал повідомлено: пляму не вдалося вивести";
    }
  }

  rclcpp_action::Server<ExecuteTask>::SharedPtr server_;
  std::mutex world_mutex_;
  int clean_attempts_{0};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::executors::MultiThreadedExecutor executor;
  auto node = std::make_shared<RoomServiceExecutor>();
  executor.add_node(node);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
