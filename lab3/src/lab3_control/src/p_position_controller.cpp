// Контролер ros2_control: П-регулятор положення суглоба з необов'язковою
// компенсацією затримки контуру (лінійний предиктор за виміряною швидкістю).
// Захоплює command interface <joint>/velocity, читає <joint>/position і <joint>/velocity.

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "controller_interface/controller_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "lab3_control/joint_sim.hpp"
#include "pluginlib/class_list_macros.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"

namespace lab3_control
{
using CallbackReturn = controller_interface::CallbackReturn;

class PPositionController : public controller_interface::ControllerInterface
{
public:
  CallbackReturn on_init() override
  {
    auto_declare<std::string>("joint", "joint1");
    auto_declare<double>("kp", 5.0);
    auto_declare<double>("target", 0.05);
    auto_declare<bool>("compensate", false);
    auto_declare<double>("loop_delay", 0.15);
    return CallbackReturn::SUCCESS;
  }

  controller_interface::InterfaceConfiguration command_interface_configuration() const override
  {
    return {controller_interface::interface_configuration_type::INDIVIDUAL,
      {joint_ + "/" + hardware_interface::HW_IF_VELOCITY}};
  }

  controller_interface::InterfaceConfiguration state_interface_configuration() const override
  {
    return {controller_interface::interface_configuration_type::INDIVIDUAL,
      {joint_ + "/" + hardware_interface::HW_IF_POSITION,
        joint_ + "/" + hardware_interface::HW_IF_VELOCITY}};
  }

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    joint_ = get_node()->get_parameter("joint").as_string();
    state_pub_ = get_node()->create_publisher<std_msgs::msg::Float64MultiArray>(
      "~/state", rclcpp::SystemDefaultsQoS());
    target_sub_ = get_node()->create_subscription<std_msgs::msg::Float64>(
      "~/target", 10, [this](const std_msgs::msg::Float64 & msg) {
        target_.store(msg.data);
        RCLCPP_INFO(get_node()->get_logger(), "Нова ціль: %.4f рад", msg.data);
      });
    RCLCPP_INFO(get_node()->get_logger(), "Контролер сконфігуровано для '%s'", joint_.c_str());
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    // Параметри зчитуються при кожній активації: їх можна змінити ros2 param set
    ctrl_.kp = get_node()->get_parameter("kp").as_double();
    ctrl_.compensate = get_node()->get_parameter("compensate").as_bool();
    ctrl_.loop_delay = get_node()->get_parameter("loop_delay").as_double();
    target_.store(get_node()->get_parameter("target").as_double());
    elapsed_ = 0.0;
    RCLCPP_INFO(
      get_node()->get_logger(), "АКТИВОВАНО: Kp=%.2f, ціль=%.3f рад, компенсація=%s (%.0f мс)",
      ctrl_.kp, target_.load(), ctrl_.compensate ? "так" : "ні", ctrl_.loop_delay * 1000);
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    // Звільняючи привід, залишаємо безпечну нульову команду
    (void)command_interfaces_[0].set_value(0.0);
    RCLCPP_INFO(get_node()->get_logger(), "ДЕАКТИВОВАНО: команда швидкості = 0");
    return CallbackReturn::SUCCESS;
  }

  // Викликається ControllerManager з частотою update_rate (T = 1 / update_rate)
  controller_interface::return_type update(const rclcpp::Time &, const rclcpp::Duration & period)
  override
  {
    elapsed_ += period.seconds();
    const auto x = state_interfaces_[0].get_optional();
    const auto v = state_interfaces_[1].get_optional();
    if (!x || !v) {
      return controller_interface::return_type::OK;  // дані тимчасово недоступні
    }
    ctrl_.target = target_.load();
    const double x_pred = ctrl_.predict(*x, *v);
    const double u = ctrl_.compute(*x, *v);
    if (!command_interfaces_[0].set_value(u)) {
      return controller_interface::return_type::ERROR;
    }

    // Діагностика: [t, x_meas, v_meas, x_pred, e, u]
    std_msgs::msg::Float64MultiArray msg;
    msg.data = {elapsed_, *x, *v, x_pred, ctrl_.target - x_pred, u};
    state_pub_->publish(msg);
    return controller_interface::return_type::OK;
  }

private:
  std::string joint_{"joint1"};
  PController ctrl_;
  std::atomic<double> target_{0.05};
  double elapsed_{0.0};
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr state_pub_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr target_sub_;
};

}  // namespace lab3_control

PLUGINLIB_EXPORT_CLASS(lab3_control::PPositionController, controller_interface::ControllerInterface)
