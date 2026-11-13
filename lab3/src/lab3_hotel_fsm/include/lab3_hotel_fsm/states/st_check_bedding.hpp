#pragma once

namespace lab3_hotel_fsm
{
// Перевірка наявності подушок і простирадл
struct StCheckBedding : smacc2::SmaccState<StCheckBedding, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvCbSuccess<CbExecuteTask, OrRobot>, StCleanStain, SUCCESS>,
    Transition<EvCbFailure<CbExecuteTask, OrRobot>, StRestockBedding, ABORT>
  > reactions;

  static void staticConfigure()
  {
    configure_orthogonal<OrRobot, CbExecuteTask>("check_bedding");
  }

  void onEntry() {RCLCPP_INFO(getLogger(), "[FSM] >>> StCheckBedding");}
};

// Поповнення відсутніх предметів (перелік з глобальних даних автомата)
struct StRestockBedding : smacc2::SmaccState<StRestockBedding, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvCbSuccess<CbExecuteTask, OrRobot>, StCleanStain, SUCCESS>,
    Transition<EvCbFailure<CbExecuteTask, OrRobot>, StMissionFailed, ABORT>
  > reactions;

  static void staticConfigure()
  {
    configure_orthogonal<OrRobot, CbExecuteTask>("restock_bedding", "", "check_bedding_data");
  }

  void onEntry() {RCLCPP_INFO(getLogger(), "[FSM] >>> StRestockBedding");}
};
}  // namespace lab3_hotel_fsm
