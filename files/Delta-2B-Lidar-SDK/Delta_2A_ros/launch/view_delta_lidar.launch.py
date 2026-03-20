from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    pkg_share = get_package_share_directory('delta_lidar')
    launch_file = os.path.join(pkg_share, 'launch', 'delta_lidar.launch.py')

    delta_lidar_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(launch_file)
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        arguments=['-d', os.path.join(pkg_share, 'rviz', 'delta_lidar.rviz')],
        output='screen'
    )

    return LaunchDescription([
        delta_lidar_launch,
        rviz_node
    ])
