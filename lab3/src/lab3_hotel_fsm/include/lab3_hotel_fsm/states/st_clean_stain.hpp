#pragma once

namespace lab3_hotel_fsm
{
// Одна спроба видалення плями
struct StCleanStain : smacc2::SmaccState<StCleanStain, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvCbSuccess<CbExecuteTask, OrRobot>, StReplaceLinen, SUCCESS>,
    Transition<EvCbFailure<CbExecuteTask, OrRobot>, StCleanStainRetry, ABORT>
  > reactions;

  static void staticConfigure()
  {
    configure_orthogonal<OrRobot, CbExecuteTask>("clean_stain");
  }

  void onEntry()
  {
    int attempts = 0;
    getGlobalSMData("clean_attempts", attempts);
    setGlobalSMData("clean_attempts", ++attempts);
    RCLCPP_INFO(getLogger(), "[FSM] >>> StCleanStain (спроба %d)", attempts);
  }
};

// Стан-розгалуження: чи можна зробити ще одну спробу
struct StCleanStainRetry : smacc2::SmaccState<StCleanStainRetry, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvLoopContinue<StCleanStainRetry>, StCleanStain, CONTINUELOOP>,
    Transition<EvLoopEnd<StCleanStainRetry>, StReportIssue, ENDLOOP>
  > reactions;

  bool canRetry()
  {
    int attempts = 0;
    getGlobalSMData("clean_attempts", attempts);
    const auto max_attempts = getNode()->get_parameter("max_clean_attempts").as_int();
    RCLCPP_INFO(
      getLogger(), "[FSM] >>> StCleanStainRetry: використано %d з %ld спроб",
      attempts, max_attempts);
    return attempts < max_attempts;
  }

  void onEntry() {checkWhileLoopConditionAndThrowEvent(&StCleanStainRetry::canRetry);}
};

// Пляму не вдалося вивести: повідомляємо персонал
struct StReportIssue : smacc2::SmaccState<StReportIssue, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvCbSuccess<CbExecuteTask, OrRobot>, StMissionFailed, SUCCESS>,
    Transition<EvCbFailure<CbExecuteTask, OrRobot>, StMissionFailed, ABORT>
  > reactions;

  static void staticConfigure()
  {
    configure_orthogonal<OrRobot, CbExecuteTask>("report_issue");
  }

  void onEntry() {RCLCPP_INFO(getLogger(), "[FSM] >>> StReportIssue");}
};
}  // namespace lab3_hotel_fsm
