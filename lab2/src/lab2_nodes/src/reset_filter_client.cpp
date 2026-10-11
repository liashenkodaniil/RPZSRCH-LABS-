#include <chrono>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "lab2_interfaces/srv/reset_filter.hpp"

using ResetFilter = lab2_interfaces::srv::ResetFilter;
using namespace std::chrono_literals;

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("reset_filter_client");
  auto client = node->create_client<ResetFilter>("reset_filter");

  // Інтроспекція і на стороні клієнта
  client->configure_introspection(
    node->get_clock(), rclcpp::SystemDefaultsQoS(), RCL_SERVICE_INTROSPECTION_CONTENTS);

  // Фоновий потік обробляє відповіді, поки головний потік чекає на future
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread spin_thread([&executor]() {executor.spin();});

  int exit_code = 0;

  if (!client->wait_for_service(3s)) {
    RCLCPP_ERROR(node->get_logger(), "Сервіс reset_filter недоступний");
    exit_code = 1;
  } else {
    auto request = std::make_shared<ResetFilter::Request>();
    auto result = client->async_send_request(request);

    // Обробка таймауту очікування відповіді
    if (result.future.wait_for(5s) == std::future_status::ready) {
      auto response = result.future.get();
      RCLCPP_INFO(
        node->get_logger(), "Відповідь: success=%s, message='%s'",
        response->success ? "true" : "false", response->message.c_str());
    } else {
      RCLCPP_ERROR(node->get_logger(), "Таймаут: відповіді не отримано за 5 секунд");
      client->remove_pending_request(result.request_id);
      exit_code = 2;
    }
  }

  executor.cancel();
  spin_thread.join();
  rclcpp::shutdown();
  return exit_code;
}