cmake_minimum_required(VERSION 3.20)
project(lab2_nodes)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(lab2_interfaces REQUIRED)

# 1. Вузли пунктів 1-5
add_executable(data_filter_node src/data_filter_node.cpp)
target_link_libraries(data_filter_node PUBLIC
  rclcpp::rclcpp
  ${std_msgs_TARGETS}
  ${lab2_interfaces_TARGETS}
)

add_executable(reset_client_node src/reset_client_node.cpp)
target_link_libraries(reset_client_node PUBLIC
  rclcpp::rclcpp
  ${lab2_interfaces_TARGETS}
)

# 2. Action-сервер (пункт 6)
add_executable(array_action_server src/array_action_server.cpp)
target_link_libraries(array_action_server PUBLIC
  rclcpp::rclcpp
  rclcpp_action::rclcpp_action
  ${lab2_interfaces_TARGETS}
)

install(TARGETS
  data_filter_node
  reset_client_node
  array_action_server
  DESTINATION lib/${PROJECT_NAME}
)

# 3. Компоненти TypeAdapter (пункт 7*)
add_library(pc_talker_component SHARED src/pc_talker_component.cpp)
target_include_directories(pc_talker_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(pc_talker_component PUBLIC
  rclcpp::rclcpp
  rclcpp_components::component
  ${sensor_msgs_TARGETS}
)
rclcpp_components_register_nodes(pc_talker_component "lab2_ns::PointCloudTalker")

add_library(pc_listener_component SHARED src/pc_listener_component.cpp)
target_include_directories(pc_listener_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(pc_listener_component PUBLIC
  rclcpp::rclcpp
  rclcpp_components::component
  ${sensor_msgs_TARGETS}
)
rclcpp_components_register_nodes(pc_listener_component "lab2_ns::PointCloudListener")

install(TARGETS
  pc_talker_component
  pc_listener_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)

ament_package()