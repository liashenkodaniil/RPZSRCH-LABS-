// Ортогональ: місце, де "живе" клієнт виконавця навичок
#pragma once

#include "lab3_hotel_fsm/clients/cl_robot_tasks.hpp"
#include "smacc2/smacc_orthogonal.hpp"

namespace lab3_hotel_fsm
{
class OrRobot : public smacc2::Orthogonal<OrRobot>
{
public:
  void onInitialize() override { this->createClient<ClRobotTasks>(); }
};
}  // namespace lab3_hotel_fsm
