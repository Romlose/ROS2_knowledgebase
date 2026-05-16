# Используем базовый официальный образ ROS в контейнере
FROM ros:humble-ros-base

# Объявляем переменные окружения
ENV ROS_DISTRO=humble
ENV ROS_DOMAIN_ID=10
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

# Устанавливаем рабочую директорию
WORKDIR /ros2_ws

# Копируем ТОЛЬКО исходники нашего пакета, при завершении работы контейнера, изменения не сохранятся
COPY src/ ./src/

# Устанавливаем зависимости и дополнительные пакеты
RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
    ros-${ROS_DISTRO}-rviz2 \
    ros-${ROS_DISTRO}-rmw-cyclonedds-cpp \
    python3-colcon-common-extensions && \
    # Очищаем кэш apt, чтобы уменьшить размер итогового слоя образа, хороший тон
    rm -rf /var/lib/apt/lists/*

#Собираем наш workspace прямо внутри образа
# Важно: перед сборкой нужно сурснуть ROS, чтобы colcon нашел ROS-зависимости
RUN . /opt/ros/${ROS_DISTRO}/setup.sh && \
    colcon build

# Настраиваем команду запуска по умолчанию
# Открываем bash, чтобы мы могли сразу вводить команды ROS2
CMD ["bash"]