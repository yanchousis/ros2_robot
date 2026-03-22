import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    pkg_description = get_package_share_directory('ros2_robot_description')
    xacro_file = os.path.join(pkg_description, 'urdf', 'ros2_robot.urdf.xacro')
    
    pkg_ros2_robot = get_package_share_directory('ros2_robot')
    rviz_config_file = os.path.join(pkg_ros2_robot, 'config', 'rviz_params.rviz')

    slam_params_file = '/home/user/ros2_robot/src/ros2_robot/config/slam_param.yaml'

    serial_port_arg = DeclareLaunchArgument(
        'serial_port',
        default_value='/dev/ttyUSB0',
        description='Serial port for lidar'
    )

    robot_description_substitution = Command(['xacro ', xacro_file])
    robot_description = ParameterValue(robot_description_substitution, value_type=str)
    robot_description_param = {'robot_description': robot_description}

    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[robot_description_param]
    )

    joint_state_publisher_node = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        name='joint_state_publisher',
        output='screen',
    )

    odom_node = Node(
        package='ros2_robot_odom',
        executable='odom_node',
        name='odom_node',
        output='screen',
    )

    lidar_node = Node(
        package='lidar_node',
        executable='lidar_node_node',
        name='lidar_node',
        parameters=[{
            'serial_port': LaunchConfiguration('serial_port'),
        }],
        output='screen',
    )

    slam_toolbox = ExecuteProcess(
        cmd=['ros2', 'launch', 'slam_toolbox', 'online_sync_launch.py',
             f'slam_params_file:={slam_params_file}'],
        output='screen'
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_file],
        output='screen'
    )

    return LaunchDescription([
        serial_port_arg,
        robot_state_publisher_node,
        joint_state_publisher_node,
        odom_node,
        lidar_node,
        slam_toolbox,
        rviz_node,
    ])
