#ifndef MY_LAB_PACKAGE__SIMPLE_COMPONENT_HPP_
#define MY_LAB_PACKAGE__SIMPLE_COMPONENT_HPP_

#include "rclcpp/rclcpp.hpp"

namespace my_lab_package
{

class SimpleComponent : public rclcpp::Node
{
public:
  explicit SimpleComponent(const rclcpp::NodeOptions & options)
  : Node("simple_component", options)
  {
    // Конструктор поки порожній, як і вимагається у завданні
  }
};

}  // namespace my_lab_package

#endif