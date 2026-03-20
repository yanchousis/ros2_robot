'''
Parameter Description:
---
- Set laser scan directon: 
  1. Set counterclockwise, example: {'laser_scan_dir': True}
  2. Set clockwise,        example: {'laser_scan_dir': False}
- Angle crop setting, Mask data within the set angle range:
  1. Enable angle crop fuction:
    1.1. enable angle crop,  example: {'enable_angle_crop_func': True}
    1.2. disable angle crop, example: {'enable_angle_crop_func': False}
  2. Angle cropping interval setting:
  - The distance and intensity data within the set angle range will be set to 0.
  - angle >= 'angle_crop_min' and angle <= 'angle_crop_max' which is [angle_crop_min, angle_crop_max], unit is degress.
    example:
      {'angle_crop_min': 135.0}
      {'angle_crop_max': 225.0}
      which is [135.0, 225.0], angle unit is degress.
'''

from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    serial_port_arg = DeclareLaunchArgument(
        'serial_port',
        default_value='/dev/ttyUSB0',
        description='Serial port of the lidar'
    )

    serial_port = LaunchConfiguration('serial_port')

    lidar_node = Node(
        package='lidar_node',
        executable='lidar_node',
        name='lidar_publisher',
        output='screen',
        parameters=[
            {'laser_scan_topic_name': 'scan'}
            {'serial_port': serial_port},
            {'frame_id': 'lidar_link'}
#         {'laser_scan_dir': True},
#         {'enable_angle_crop_func': False},
#         {'angle_crop_min': 135.0},  # unit is degress
#         {'angle_crop_max': 225.0},  # unit is degress
#         {'range_min': 0.02}, # unit is meter
#         {'range_max': 12.0}   # unit is meter
        ]
    )

    return LaunchDescription([
        serial_port_arg,
        lidar_node
    ])