from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    season = LaunchConfiguration('season')
    return LaunchDescription([
        DeclareLaunchArgument('season', default_value='winter'),
        Node(package='lab3_hotel', executable='room_service_executor', output='screen'),
        Node(package='lab3_hotel', executable='bt_orchestrator', output='screen',
             parameters=[{'season': season}]),
    ])
