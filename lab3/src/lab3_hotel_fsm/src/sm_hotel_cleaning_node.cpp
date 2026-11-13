#include "lab3_hotel_fsm/sm_hotel_cleaning.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  smacc2::run<lab3_hotel_fsm::SmHotelCleaning>();
}
