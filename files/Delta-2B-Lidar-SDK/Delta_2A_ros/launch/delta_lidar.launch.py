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

    delta_lidar_node = Node(
        package='delta_lidar',
        executable='delta_lidar_node',
        name='delta_lidar',
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
        delta_lidar_node
    ])
