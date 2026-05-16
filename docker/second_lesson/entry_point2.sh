#!/bin/bash
set -e

# 1. Подключаем базовое окружение ROS 2, записываем в bashrc и сурсим его
cat >> ~/.bashrc "source /opt/ros/${ROS_DISTRO}/setup.bash"
cat >> ~/.bashrc "source install/setup.bash"
source ~/.bashrc

# 2. Подключаем наше рабочее пространство (если оно было собрано в Dockerfile)
# Проверяем наличие файла, чтобы контейнер не упал, если мы собираем на лету
if [ -f /ros2_ws/install/setup.bash ]; then
    source /ros2_ws/install/setup.bash
    echo "Рабочее пространство /ros2_ws успешно подключено."
fi

# 3. Выполняем переданную команду из docker-compose или docker run
# Команда exec заменяет текущий процесс bash на целевой, 
# что КРИТИЧЕСКИ ВАЖНО для правильной остановки контейнера
exec "$@"