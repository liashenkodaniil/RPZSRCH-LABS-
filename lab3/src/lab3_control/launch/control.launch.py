import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    share = get_package_share_directory('lab3_control')
    with open(os.path.join(share, 'urdf', 'single_joint.urdf')) as f:
        robot_description = f.read()
    controllers = os.path.join(share, 'config', 'controllers.yaml')

    return LaunchDescription([
        Node(package='robot_state_publisher', executable='robot_state_publisher',
             parameters=[{'robot_description': robot_description}]),
        Node(package='controller_manager', executable='ros2_control_node',
             parameters=[controllers], output='screen'),
        Node(package='controller_manager', executable='spawner',
             arguments=['joint_state_broadcaster']),
        # Звичайний П-регулятор: завантажено й активовано
        Node(package='controller_manager', executable='spawner',
             arguments=['p_controller']),
        # П-регулятор з компенсацією: завантажено, але неактивний
        Node(package='controller_manager', executable='spawner',
             arguments=['p_controller_compensated', '--inactive']),
    ])
