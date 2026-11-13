#pragma once

namespace lab3_hotel_fsm
{
// Заміна постільної білизни
struct StReplaceLinen : smacc2::SmaccState<StReplaceLinen, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvCbSuccess<CbExecuteTask, OrRobot>, StSelectBlanket, SUCCESS>,
    Transition<EvCbFailure<CbExecuteTask, OrRobot>, StMissionFailed, ABORT>
  > reactions;

  static void staticConfigure()
  {
    configure_orthogonal<OrRobot, CbExecuteTask>("replace_linen");
  }

  void onEntry() {RCLCPP_INFO(getLogger(), "[FSM] >>> StReplaceLinen");}
};
}  // namespace lab3_hotel_fsm
