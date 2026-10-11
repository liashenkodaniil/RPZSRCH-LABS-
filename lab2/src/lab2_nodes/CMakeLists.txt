cmake_minimum_required(VERSION 3.8)
project(lab2_nodes)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(std_msgs REQUIRED)
find_package(lab2_interfaces REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(PCL REQUIRED COMPONENTS common)

add_library(pcl_components SHARED
  src/pcl_publisher.cpp
  src/pcl_subscriber.cpp)

target_include_directories(pcl_components PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_target_dependencies(pcl_components
  rclcpp rclcpp_components sensor_msgs pcl_conversions)
target_link_libraries(pcl_components ${PCL_LIBRARIES})

rclcpp_components_register_nodes(pcl_components
  "lab2_nodes::PclPublisher"
  "lab2_nodes::PclSubscriber")

add_executable(data_filter src/data_filter.cpp)
add_executable(reset_filter_client src/reset_filter_client.cpp)
ament_target_dependencies(data_filter rclcpp rclcpp_action std_msgs lab2_interfaces)
ament_target_dependencies(reset_filter_client rclcpp lab2_interfaces)
add_executable(process_array_server src/process_array_server.cpp)
ament_target_dependencies(process_array_server rclcpp rclcpp_action lab2_interfaces)

install(TARGETS pcl_components
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(TARGETS data_filter reset_filter_client process_array_server
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY include/ DESTINATION include)

ament_package()