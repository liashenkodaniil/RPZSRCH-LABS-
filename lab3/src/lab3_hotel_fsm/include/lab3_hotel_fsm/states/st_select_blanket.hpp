#pragma once

#include <string>

namespace lab3_hotel_fsm
{
// Стан-розгалуження за сезоном: генерує одну з трьох подій
struct StSelectBlanket : smacc2::SmaccState<StSelectBlanket, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvWinter, StReplaceWarmDuvet, SUCCESS>,
    Transition<EvSummer, StReplaceLightBlanket, SUCCESS>,
    Transition<EvSpringAutumn, StReplaceAllSeasonDuvet, SUCCESS>
  > reactions;

  void onEntry()
  {
    const auto season = getNode()->get_parameter("season").as_string();
    RCLCPP_INFO(getLogger(), "[FSM] >>> StSelectBlanket: сезон '%s'", season.c_str());
    if (season == "winter") {
      this->postEvent<EvWinter>();
    } else if (season == "summer") {
      this->postEvent<EvSummer>();
    } else {
      this->postEvent<EvSpringAutumn>();
    }
  }
};

// Окремий стан для кожного варіанта ковдри (класичний підхід FSM)
struct StReplaceWarmDuvet : smacc2::SmaccState<StReplaceWarmDuvet, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvCbSuccess<CbExecuteTask, OrRobot>, StMissionComplete, SUCCESS>,
    Transition<EvCbFailure<CbExecuteTask, OrRobot>, StMissionFailed, ABORT>
  > reactions;

  static void staticConfigure()
  {
    configure_orthogonal<OrRobot, CbExecuteTask>("replace_blanket", "warm_duvet");
  }

  void onEntry() {RCLCPP_INFO(getLogger(), "[FSM] >>> StReplaceWarmDuvet");}
};

struct StReplaceLightBlanket : smacc2::SmaccState<StReplaceLightBlanket, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvCbSuccess<CbExecuteTask, OrRobot>, StMissionComplete, SUCCESS>,
    Transition<EvCbFailure<CbExecuteTask, OrRobot>, StMissionFailed, ABORT>
  > reactions;

  static void staticConfigure()
  {
    configure_orthogonal<OrRobot, CbExecuteTask>("replace_blanket", "light_blanket");
  }

  void onEntry() {RCLCPP_INFO(getLogger(), "[FSM] >>> StReplaceLightBlanket");}
};

struct StReplaceAllSeasonDuvet : smacc2::SmaccState<StReplaceAllSeasonDuvet, SmHotelCleaning>
{
  using SmaccState::SmaccState;

  typedef mpl::list<
    Transition<EvCbSuccess<CbExecuteTask, OrRobot>, StMissionComplete, SUCCESS>,
    Transition<EvCbFailure<CbExecuteTask, OrRobot>, StMissionFailed, ABORT>
  > reactions;

  static void staticConfigure()
  {
    configure_orthogonal<OrRobot, CbExecuteTask>("replace_blanket", "all_season_duvet");
  }

  void onEntry() {RCLCPP_INFO(getLogger(), "[FSM] >>> StReplaceAllSeasonDuvet");}
};
}  // namespace lab3_hotel_fsm
