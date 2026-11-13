#pragma once

namespace lab3_hotel_fsm
{
// Кінцеві стани місії
struct StMissionComplete : smacc2::SmaccState<StMissionComplete, SmHotelCleaning>
{
  using SmaccState::SmaccState;
  typedef mpl::list<> reactions;

  void onEntry() {RCLCPP_INFO(getLogger(), "[FSM] >>> StMissionComplete: номер прибрано");}
};

struct StMissionFailed : smacc2::SmaccState<StMissionFailed, SmHotelCleaning>
{
  using SmaccState::SmaccState;
  typedef mpl::list<> reactions;

  void onEntry() {RCLCPP_ERROR(getLogger(), "[FSM] >>> StMissionFailed: місію не виконано");}
};
}  // namespace lab3_hotel_fsm
