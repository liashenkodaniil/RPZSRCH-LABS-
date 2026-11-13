// Поведінка клієнта: асинхронно виконує одну навичку виконавця і генерує
// подію EvCbSuccess / EvCbFailure, за якою автомат здійснює перехід.
#pragma once

#include <memory>
#include <string>

#include "smacc2/smacc_asynchronous_client_behavior.hpp"
#include "lab3_hotel_fsm/clients/cl_robot_tasks.hpp"

namespace lab3_hotel_fsm
{
class CbExecuteTask : public smacc2::SmaccAsyncClientBehavior
{
public:
  // task -- назва навички; argument -- її аргумент;
  // argument_key -- якщо задано, аргумент береться з глобальних даних автомата
  explicit CbExecuteTask(
    std::string task, std::string argument = "", std::string argument_key = "")
  : task_(std::move(task)), argument_(std::move(argument)), argument_key_(std::move(argument_key))
  {
  }

  template<typename TOrthogonal, typename TSourceObject>
  void onStateOrthogonalAllocation()
  {
    this->requiresClient(client_);
    smacc2::SmaccAsyncClientBehavior::onStateOrthogonalAllocation<TOrthogonal, TSourceObject>();
  }

  void onEntry() override
  {
    // onEntry асинхронної поведінки виконується в окремому потоці,
    // тому очікування сервера не блокує автомат
    client_->waitForServer();

    ClRobotTasks::Goal goal;
    goal.task = task_;
    goal.argument = argument_;
    if (!argument_key_.empty()) {
      this->getStateMachine()->getGlobalSMData(argument_key_, goal.argument);
    }

    RCLCPP_INFO(
      getLogger(), "[FSM] Ціль виконавцю: task='%s' argument='%s'",
      goal.task.c_str(), goal.argument.c_str());

    result_signal_ = std::make_shared<ClRobotTasks::TaskResultSignal>();
    this->getStateMachine()->createSignalConnection(
      *result_signal_, &CbExecuteTask::onResult, this);
    client_->sendGoal(goal, result_signal_);
  }

private:
  void onResult(const ClRobotTasks::WrappedResult & result)
  {
    const bool ok = result.code == rclcpp_action::ResultCode::SUCCEEDED &&
      result.result && result.result->success;
    if (result.result) {
      // Дані результату зберігаються в глобальних даних автомата (аналог Blackboard)
      this->getStateMachine()->setGlobalSMData(task_ + "_data", result.result->data);
      RCLCPP_INFO(
        getLogger(), "[FSM] %s -> %s: %s", task_.c_str(), ok ? "SUCCESS" : "FAILURE",
        result.result->message.c_str());
    }
    if (ok) {
      this->postSuccessEvent();
    } else {
      this->postFailureEvent();
    }
  }

  std::string task_;
  std::string argument_;
  std::string argument_key_;
  ClRobotTasks * client_ = nullptr;
  ClRobotTasks::TaskResultSignal::SharedPtr result_signal_;
};
}  // namespace lab3_hotel_fsm
