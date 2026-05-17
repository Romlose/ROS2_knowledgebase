cmake_minimum_required(VERSION 3.11)
project(ros2_course)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")

# --- BASIC_GROUP ---
add_executable(first src/first_lesson.cpp)
ament_target_dependencies(first rclcpp)
install(TARGETS first DESTINATION lib/${PROJECT_NAME})

add_executable(third_1 src/third_lesson_1.cpp)
ament_target_dependencies(third_1 rclcpp)
install(TARGETS third_1 DESTINATION lib/${PROJECT_NAME})

add_executable(third_2 src/third_lesson_2.cpp)
ament_target_dependencies(third_2 rclcpp)
install(TARGETS third_2 DESTINATION lib/${PROJECT_NAME})

add_executable(wait_set_keyboard src/WaitSet.cpp)
ament_target_dependencies(wait_set_keyboard rclcpp)
install(TARGETS wait_set_keyboard DESTINATION lib/${PROJECT_NAME})

# --- MSGS_UNIQUE_GROUP ---
add_executable(second src/second_lesson.cpp)
ament_target_dependencies(second rclcpp std_msgs sensor_msgs)
install(TARGETS second DESTINATION lib/${PROJECT_NAME})

add_executable(fourth src/fourth_lesson.cpp)
ament_target_dependencies(fourth rclcpp std_msgs sensor_msgs)
install(TARGETS fourth DESTINATION lib/${PROJECT_NAME})

# --- OPENCV_GROUP ---
#add_executable(opencv_processor src/opencv_processor.cpp)
#ament_target_dependencies(opencv_processor rclcpp sensor_msgs cv_bridge image_transport)
#install(TARGETS opencv_processor DESTINATION lib/${PROJECT_NAME})

ament_package()