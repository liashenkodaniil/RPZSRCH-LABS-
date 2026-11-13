cmake_minimum_required(VERSION 3.8)
project(lab3_hotel)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(lab3_interfaces REQUIRED)

# Виконавець навичок (сервер дій)
add_executable(room_service_executor src/room_service_executor.cpp)
ament_target_dependencies(room_service_executor rclcpp rclcpp_action lab3_interfaces)

# Оркестратор місії на дереві поведінки
add_executable(bt_orchestrator src/bt_orchestrator.cpp)
ament_target_dependencies(bt_orchestrator
  rclcpp rclcpp_action behaviortree_cpp ament_index_cpp lab3_interfaces)

install(TARGETS room_service_executor bt_orchestrator
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY trees launch
  DESTINATION share/${PROJECT_NAME})

ament_package()
