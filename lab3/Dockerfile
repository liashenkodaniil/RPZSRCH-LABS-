FROM osrf/ros:jazzy-desktop

# Залежності лабораторної роботи №3:
#  - BehaviorTree.CPP v4 (дерева поведінки)
#  - SMACC2 (скінченні автомати)
#  - ros2_control + ros2_controllers (ControllerManager, JointStateBroadcaster, CLI)
RUN apt-get update && apt-get install -y --no-install-recommends \
      ros-jazzy-behaviortree-cpp \
      ros-jazzy-smacc2 \
      ros-jazzy-ros2-control \
      ros-jazzy-ros2-controllers \
      python3-matplotlib \
    && rm -rf /var/lib/apt/lists/*

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

WORKDIR /ros2_ws
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
