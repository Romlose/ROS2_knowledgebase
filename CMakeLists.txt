cmake_minimum_required(VERSION 3.11)
project(ros2_course)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# зависимости OpenCV
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")

add_executable(pub src/third_lesson_2.cpp)

ament_target_dependencies(pub
  rclcpp
  std_msgs
)

# добавляем исполняемые файлы для OpenCV
add_executable(opencv_processor src/opencv_processor.cpp)

ament_target_dependencies(opencv_processor
  rclcpp
  sensor_msgs
  cv_bridge
  image_transport
)

install(TARGETS
  pub
  opencv_processor
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()