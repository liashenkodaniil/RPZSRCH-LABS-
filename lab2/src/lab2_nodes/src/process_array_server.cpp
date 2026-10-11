#include <algorithm>
#include <cctype>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "lab2_interfaces/action/process_array.hpp"

using ProcessArray = lab2_interfaces::action::ProcessArray;
using GoalHandle = rclcpp_action::ServerGoalHandle<ProcessArray>;
using namespace std::chrono_literals;

class ProcessArrayServer : public rclcpp::Node
{
public:
  ProcessArrayServer()
  : Node("process_array_server")
  {
    using namespace std::placeholders;
    server_ = rclcpp_action::create_server<ProcessArray>(
      this, "process_array",
      std::bind(&ProcessArrayServer::handle_goal, this, _1, _2),
      std::bind(&ProcessArrayServer::handle_cancel, this, _1),
      std::bind(&ProcessArrayServer::handle_accepted, this, _1));
    RCLCPP_INFO(get_logger(), "Action-сервер process_array запущено");
  }

private:
  // 1) Рішення: приймати ціль чи ні
  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID &,
    std::shared_ptr<const ProcessArray::Goal> goal)
  {
    if (goal->data.empty()) {
      RCLCPP_WARN(get_logger(), "Ціль відхилено: порожній масив");
      return rclcpp_action::GoalResponse::REJECT;
    }
    RCLCPP_INFO(get_logger(), "Ціль прийнято, елементів: %zu", goal->data.size());
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  // 2) Рішення: чи дозволяти скасування
  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandle>)
  {
    RCLCPP_INFO(get_logger(), "Отримано запит на скасування");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  // 3) Ціль прийнято: запускаємо виконання в ОКРЕМОМУ потоці,
  //    щоб не блокувати executor
  void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle)
  {
    std::thread{
      std::bind(&ProcessArrayServer::execute, this, std::placeholders::_1),
      goal_handle}.detach();
  }

  void execute(const std::shared_ptr<GoalHandle> goal_handle)
  {
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<ProcessArray::Feedback>();
    auto result = std::make_shared<ProcessArray::Result>();

    const uint32_t total = static_cast<uint32_t>(goal->data.size());
    feedback->total = total;

    for (uint32_t i = 0; i < total; ++i) {
      // Перевірка скасування на кожній ітерації
      if (goal_handle->is_canceling()) {
        result->success = false;
        goal_handle->canceled(result);
        RCLCPP_INFO(get_logger(), "Ціль скасовано на елементі %u з %u", i, total);
        return;
      }

      std::string item = goal->data[i];
      std::transform(
        item.begin(), item.end(), item.begin(),
        [](unsigned char c) {return static_cast<char>(std::toupper(c));});
      result->processed.push_back(item);

      // Періодичний feedback про поточний стан
      feedback->processed_count = i + 1;
      feedback->state = "Processing";
      goal_handle->publish_feedback(feedback);

      std::this_thread::sleep_for(500ms);  // імітація тривалої обробки
    }

    // Фінальний feedback зі станом Succeeded, потім результат
    feedback->state = "Succeeded";
    goal_handle->publish_feedback(feedback);

    result->success = true;
    goal_handle->succeed(result);
    RCLCPP_INFO(get_logger(), "Ціль виконано успішно");
  }

  rclcpp_action::Server<ProcessArray>::SharedPtr server_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ProcessArrayServer>());
  rclcpp::shutdown();
  return 0;
}