cmake_minimum_required(VERSION 3.10)
project(lab3_hotel_fsm)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(smacc2 REQUIRED)
find_package(lab3_interfaces REQUIRED)
find_package(Boost COMPONENTS thread REQUIRED)

add_executable(sm_hotel_cleaning_node src/sm_hotel_cleaning_node.cpp)
target_include_directories(sm_hotel_cleaning_node PUBLIC include)
ament_target_dependencies(sm_hotel_cleaning_node
  rclcpp rclcpp_action smacc2 lab3_interfaces)
target_link_libraries(sm_hotel_cleaning_node ${Boost_LIBRARIES})

install(TARGETS sm_hotel_cleaning_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
