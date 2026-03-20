from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
  return LaunchDescription([
    Node(
      package='timer_node',
      executable='timer_node',
      name='time_node',
      output='screen'
    )
  ])