cmake_minimum_required(VERSION 3.11)
project(ros2_course)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")

add_executable(pub src/third_lesson.cpp)

ament_target_dependencies(pub
  rclcpp
  std_msgs
)

install(TARGETS
  pub
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()