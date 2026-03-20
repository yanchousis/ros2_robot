cmake_minimum_required(VERSION 3.8)
project(lidar_node)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

include_directories(
  include
  ../sdk/include
)

add_executable(lidar_node src/node.cpp)

ament_target_dependencies(lidar_node
  rclcpp
  sensor_msgs
)

add_executable(lidar_client src/client.cpp)

ament_target_dependencies(lidar_client
  rclcpp
  sensor_msgs
)

install(TARGETS
  lidar_node
  lidar_client
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch rviz
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
