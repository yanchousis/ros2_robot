cmake_minimum_required(VERSION 3.5)
project(delta_lidar)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra)
endif()

set(delta_lidar_SDK_PATH "./sdk/")

FILE(GLOB delta_lidar_SDK_SRC 
  "${delta_lidar_SDK_PATH}/src/*.cpp"
)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

include_directories(
  ${delta_lidar_SDK_PATH}/include
  ${delta_lidar_SDK_PATH}/src
)

add_executable(delta_lidar_node src/node.cpp ${delta_lidar_SDK_SRC})
ament_target_dependencies(delta_lidar_node rclcpp sensor_msgs std_msgs)
target_link_libraries(delta_lidar_node rt)

add_executable(delta_lidar_node_client src/client.cpp)
ament_target_dependencies(delta_lidar_node_client rclcpp sensor_msgs)

install(TARGETS delta_lidar_node delta_lidar_node_client
  ARCHIVE DESTINATION lib/${PROJECT_NAME}
  LIBRARY DESTINATION lib/${PROJECT_NAME}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch rviz sdk
  DESTINATION share/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()
