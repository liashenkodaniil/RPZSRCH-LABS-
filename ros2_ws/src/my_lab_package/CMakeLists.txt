cmake_minimum_required(VERSION 3.8)
project(my_lab_package)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)

# --- Наш доданий блок для Завдання 1.2 ---

add_library(simple_component SHARED
  src/simple_component.cpp)

target_include_directories(simple_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_target_dependencies(simple_component
  rclcpp
  rclcpp_components)

rclcpp_components_register_nodes(simple_component
  "my_lab_package::SimpleComponent")

install(TARGETS simple_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(DIRECTORY include/
  DESTINATION include)

# --- Наш доданий блок для Завдання 1.3 ---

add_executable(lifecycle_main src/lifecycle_main.cpp)

target_include_directories(lifecycle_main PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_target_dependencies(lifecycle_main
  rclcpp
  rclcpp_lifecycle)

install(TARGETS lifecycle_main
  DESTINATION lib/${PROJECT_NAME})

# --- Кінець доданого блоку ---

add_executable(lifecycle_supervisor src/lifecycle_supervisor.cpp)
ament_target_dependencies(lifecycle_supervisor rclcpp lifecycle_msgs)
install(TARGETS lifecycle_supervisor DESTINATION lib/${PROJECT_NAME})

# --- Кінець доданого блоку ---

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()