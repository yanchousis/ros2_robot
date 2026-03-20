from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    serial_port_arg = DeclareLaunchArgument(
        'serial_port',
        default_value='/dev/ttyUSB0',
        description='Serial port for lidar'
    )

    frame_id_arg = DeclareLaunchArgument(
        'frame_id',
        default_value='lidar_link',
        description='Frame ID for laser scan'
    )

    rotation_arg = DeclareLaunchArgument(
        'rotation',
        default_value='180',
        description='Lidar rotation offset in degrees (180 for upside-down)'
    )

    lidar_node_node = Node(
        package='lidar_node',
        executable='lidar_node_node',
        name='lidar_node',
        parameters=[{
            'serial_port': LaunchConfiguration('serial_port'),
            'frame_id': LaunchConfiguration('frame_id'),
            'rotation': LaunchConfiguration('rotation'),
        }],
        output='screen'
    )

    return LaunchDescription([
        serial_port_arg,
        frame_id_arg,
        rotation_arg,
        lidar_node_node
    ])
