FROM osrf/ros:humble-desktop

# Автоматичний сорсинг ROS-оточення при вході в контейнер
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

WORKDIR /ros2_ws
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]