#ifndef MY_LAB_PACKAGE__SIMPLE_LIFECYCLE_NODE_HPP_
#define MY_LAB_PACKAGE__SIMPLE_LIFECYCLE_NODE_HPP_

#include "rclcpp_lifecycle/lifecycle_node.hpp"

namespace my_lab_package
{

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class SimpleLifecycleNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit SimpleLifecycleNode(const std::string & node_name)
  : rclcpp_lifecycle::LifecycleNode(node_name)
  {
  }

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(get_logger(), "Перехід: on_configure виконано");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(get_logger(), "Перехід: on_activate виконано");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(get_logger(), "Перехід: on_deactivate виконано");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(get_logger(), "Перехід: on_cleanup виконано");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override
  {
    RCLCPP_INFO(get_logger(), "Перехід: on_shutdown виконано");
    return CallbackReturn::SUCCESS;
  }
};

}  // namespace my_lab_package

#endif