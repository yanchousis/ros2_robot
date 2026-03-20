from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():

    rviz_config = PathJoinSubstitution([
        FindPackageShare('lidar_node'),
        'rviz',
        'config_rviz.rviz'
    ])

    lidar_node = Node(
        package='lidar_node',
        executable='lidar_node',
        name='lidar_node',
        output='screen',
        parameters=[
            {'serial_port': '/dev/ttyUSB0'},
            {'frame_id': 'lidar_link'}
        ]
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_config]
    )

    return LaunchDescription([
        lidar_node,
        rviz_node
    ])