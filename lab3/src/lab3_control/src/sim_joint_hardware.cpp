// Апаратний компонент ros2_control (SystemInterface), що симулює привід суглоба
// з обмеженням прискорення, затримкою актуації та затримкою енкодера.
// Command interface: velocity; state interfaces: position, velocity (виміряні, із запізненням).

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "lab3_control/joint_sim.hpp"
#include "pluginlib/class_list_macros.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/state.hpp"

namespace lab3_control
{
using CallbackReturn = hardware_interface::CallbackReturn;

class SimJointHardware : public hardware_interface::SystemInterface
{
public:
  CallbackReturn on_init(const hardware_interface::HardwareInfo & info) override
  {
    if (hardware_interface::SystemInterface::on_init(info) != CallbackReturn::SUCCESS) {
      return CallbackReturn::ERROR;
    }
    if (info_.joints.size() != 1) {
      RCLCPP_ERROR(rclcpp::get_logger("SimJointHardware"), "Очікується рівно один суглоб");
      return CallbackReturn::ERROR;
    }
    auto param = [this](const std::string & name, double def) {
        auto it = info_.hardware_parameters.find(name);
        return it == info_.hardware_parameters.end() ? def : std::stod(it->second);
      };
    a_max_ = param("a_max", 0.5);
    actuation_delay_ = param("actuation_delay", 0.1);
    feedback_delay_ = param("feedback_delay", 0.05);
    sim_dt_ = param("sim_dt", 0.001);
    plant_ = std::make_unique<DelayedJointPlant>(a_max_, actuation_delay_, feedback_delay_, sim_dt_);

    RCLCPP_INFO(
      rclcpp::get_logger("SimJointHardware"),
      "Привід '%s': a_max=%.2f рад/с^2, dt_actuation=%.0f мс, dt_feedback=%.0f мс",
      info_.joints[0].name.c_str(), a_max_, actuation_delay_ * 1000, feedback_delay_ * 1000);
    return CallbackReturn::SUCCESS;
  }

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override
  {
    std::vector<hardware_interface::StateInterface> interfaces;
    interfaces.emplace_back(
      info_.joints[0].name, hardware_interface::HW_IF_POSITION, &position_);
    interfaces.emplace_back(
      info_.joints[0].name, hardware_interface::HW_IF_VELOCITY, &velocity_);
    return interfaces;
  }

  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override
  {
    std::vector<hardware_interface::CommandInterface> interfaces;
    interfaces.emplace_back(
      info_.joints[0].name, hardware_interface::HW_IF_VELOCITY, &velocity_command_);
    return interfaces;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    sim_time_ = 0.0;
    plant_->reset();
    position_ = velocity_ = velocity_command_ = 0.0;
    RCLCPP_INFO(rclcpp::get_logger("SimJointHardware"), "Привід активовано, x=0, v=0");
    return CallbackReturn::SUCCESS;
  }

  // Рух приводу до поточного моменту та видача ВИМІРЯНОГО (запізнілого) стану
  hardware_interface::return_type read(const rclcpp::Time &, const rclcpp::Duration & period)
  override
  {
    sim_time_ += period.seconds();
    plant_->advance_to(sim_time_);
    const auto meas = plant_->measure(sim_time_);
    position_ = meas.x;
    velocity_ = meas.v;
    return hardware_interface::return_type::OK;
  }

  // Команда контролера потрапляє в привід через actuation_delay
  hardware_interface::return_type write(const rclcpp::Time &, const rclcpp::Duration &) override
  {
    const double u = std::isfinite(velocity_command_) ? velocity_command_ : 0.0;
    plant_->command(sim_time_, u);
    return hardware_interface::return_type::OK;
  }

private:
  std::unique_ptr<DelayedJointPlant> plant_;
  double a_max_{0.5};
  double actuation_delay_{0.1};
  double feedback_delay_{0.05};
  double sim_dt_{0.001};
  double sim_time_{0.0};
  double position_{0.0};
  double velocity_{0.0};
  double velocity_command_{0.0};
};

}  // namespace lab3_control

PLUGINLIB_EXPORT_CLASS(lab3_control::SimJointHardware, hardware_interface::SystemInterface)
