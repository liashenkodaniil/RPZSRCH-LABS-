#include "rclcpp/rclcpp.hpp"
#include "lifecycle_msgs/srv/change_state.hpp"
#include "lifecycle_msgs/msg/transition.hpp"

using ChangeState = lifecycle_msgs::srv::ChangeState;

void trigger_transition(
  const std::shared_ptr<rclcpp::Node> & node,
  const std::string & node_name,
  uint8_t transition_id)
{
  auto client = node->create_client<ChangeState>("/" + node_name + "/change_state");
  if (!client->wait_for_service(std::chrono::seconds(5))) {
    RCLCPP_ERROR(node->get_logger(), "Сервіс %s недоступний", node_name.c_str());
    return;
  }

  auto request = std::make_shared<ChangeState::Request>();
  request->transition.id = transition_id;

  auto future = client->async_send_request(request);
  if (rclcpp::spin_until_future_complete(node, future) ==
    rclcpp::FutureReturnCode::SUCCESS)
  {
    RCLCPP_INFO(node->get_logger(), "%s: перехід виконано", node_name.c_str());
  } else {
    RCLCPP_ERROR(node->get_logger(), "%s: перехід НЕ виконано", node_name.c_str());
  }
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto supervisor = std::make_shared<rclcpp::Node>("lifecycle_supervisor");

  for (const auto & name : {"lifecycle_node_a", "lifecycle_node_b"}) {
    trigger_transition(supervisor, name, lifecycle_msgs::msg::Transition::TRANSITION_CONFIGURE);
    trigger_transition(supervisor, name, lifecycle_msgs::msg::Transition::TRANSITION_ACTIVATE);
  }

  rclcpp::shutdown();
  return 0;
}