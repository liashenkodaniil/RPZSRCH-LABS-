cmake_minimum_required(VERSION 3.10)
project(lab3_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(controller_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(std_msgs REQUIRED)

# Плагіни ros2_control: апаратний компонент + контролер
add_library(lab3_control SHARED
  src/sim_joint_hardware.cpp
  src/p_position_controller.cpp)
target_include_directories(lab3_control PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(lab3_control
  rclcpp rclcpp_lifecycle hardware_interface controller_interface pluginlib std_msgs)

pluginlib_export_plugin_description_file(hardware_interface lab3_control_hardware.xml)
pluginlib_export_plugin_description_file(controller_interface lab3_control_controllers.xml)

# Офлайн-моделювання перехідного процесу
add_executable(delay_sim src/delay_sim.cpp)
target_include_directories(delay_sim PRIVATE include)
ament_target_dependencies(delay_sim rclcpp)

install(TARGETS lab3_control
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
install(TARGETS delay_sim DESTINATION lib/${PROJECT_NAME})
install(PROGRAMS scripts/plot_results.py DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY config urdf launch DESTINATION share/${PROJECT_NAME})

ament_export_libraries(lab3_control)
ament_package()
