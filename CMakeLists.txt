cmake_minimum_required(VERSION 3.5)
project(motion_bridge)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(bfc_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

add_executable(motion_bridge src/motion_bridge.cpp)
ament_target_dependencies(motion_bridge rclcpp geometry_msgs std_msgs sensor_msgs bfc_msgs)

install(TARGETS
  motion_bridge
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
