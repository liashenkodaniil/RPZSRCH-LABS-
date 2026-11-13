// Скінченний автомат (SMACC2) місії прибирання номера готелю
#pragma once

#include <string>

#include "smacc2/smacc.hpp"

#include "lab3_hotel_fsm/clients/cl_robot_tasks.hpp"
#include "lab3_hotel_fsm/clients/client_behaviors/cb_execute_task.hpp"
#include "lab3_hotel_fsm/orthogonals/or_robot.hpp"

namespace lab3_hotel_fsm
{
namespace sc = boost::statechart;
namespace mpl = boost::mpl;
using smacc2::Transition;
using smacc2::EvCbSuccess;
using smacc2::EvCbFailure;
using smacc2::default_events::EvLoopContinue;
using smacc2::default_events::EvLoopEnd;
using namespace smacc2::default_transition_tags;

// Власні події вибору ковдри за сезоном
struct EvWinter : sc::event<EvWinter> {};
struct EvSummer : sc::event<EvSummer> {};
struct EvSpringAutumn : sc::event<EvSpringAutumn> {};

// Оголошення станів
struct StCheckBedding;
struct StRestockBedding;
struct StCleanStain;
struct StCleanStainRetry;
struct StReportIssue;
struct StReplaceLinen;
struct StSelectBlanket;
struct StReplaceWarmDuvet;
struct StReplaceLightBlanket;
struct StReplaceAllSeasonDuvet;
struct StMissionComplete;
struct StMissionFailed;

struct SmHotelCleaning
: public smacc2::SmaccStateMachineBase<SmHotelCleaning, StCheckBedding>
{
  using SmaccStateMachineBase::SmaccStateMachineBase;

  void onInitialize() override
  {
    // Параметри місії
    getNode()->declare_parameter<std::string>("season", "winter");
    getNode()->declare_parameter<int>("max_clean_attempts", 3);

    // Глобальні дані автомата (аналог Blackboard у BT)
    setGlobalSMData("clean_attempts", 0);
    setGlobalSMData("check_bedding_data", std::string(""));

    this->createOrthogonal<OrRobot>();
  }
};
}  // namespace lab3_hotel_fsm

#include "lab3_hotel_fsm/states/st_check_bedding.hpp"
#include "lab3_hotel_fsm/states/st_clean_stain.hpp"
#include "lab3_hotel_fsm/states/st_replace_linen.hpp"
#include "lab3_hotel_fsm/states/st_select_blanket.hpp"
#include "lab3_hotel_fsm/states/st_mission_end.hpp"
