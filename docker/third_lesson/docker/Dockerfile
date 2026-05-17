#устанавливаем все зависимости и собираем проект в билдере
FROM ros:humble AS builder

WORKDIR /ros2_ws/src/
COPY  ./ros2_course/ ./ros2_course

RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y python3-colcon-common-extensions && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /ros2_ws
RUN apt-get update && \
    apt install python3-opencv -y && \
    rosdep update && \
    rosdep install --from-paths src --ignore-src -r -y && \
    rm -rf /var/lib/apt/lists/*

RUN bash -c "source /opt/ros/humble/setup.bash && colcon build"

#экспортируем сборку в дефолтный контейнер
FROM ros:humble-ros-core
WORKDIR /ros2_ws/src
COPY --from=builder /ros2_ws/install /ros2_ws/install

COPY ./docker/ros_entrypoint.sh /ros_entrypoint.sh
RUN chmod +x /ros_entrypoint.sh
ENTRYPOINT [ "/ros_entrypoint.sh" ]

CMD [ "bash" ]