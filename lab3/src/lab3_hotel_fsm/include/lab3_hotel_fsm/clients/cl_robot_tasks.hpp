// SMACC2-клієнт: обгортка над action-клієнтом /execute_task вузла-виконавця
#pragma once

#include <string>

#include "lab3_interfaces/action/execute_task.hpp"
#include "smacc2/client_bases/smacc_action_client_base.hpp"
#include "smacc2/smacc.hpp"

namespace lab3_hotel_fsm
{
class ClRobotTasks
: public smacc2::client_bases::SmaccActionClientBase<lab3_interfaces::action::ExecuteTask>
{
public:
  using Base = smacc2::client_bases::SmaccActionClientBase<lab3_interfaces::action::ExecuteTask>;
  using Base::GoalHandle;
  using Base::ResultCallback;

  typedef smacc2::SmaccSignal<void(const WrappedResult &)> TaskResultSignal;

  explicit ClRobotTasks(std::string action_name = "/execute_task")
  : Base(action_name)
  {
  }

  virtual ~ClRobotTasks() {}
};
}  // namespace lab3_hotel_fsm
