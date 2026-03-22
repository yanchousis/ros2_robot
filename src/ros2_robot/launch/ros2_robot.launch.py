import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.actions import TimerAction, Shutdown, LogInfo
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    pkg_description = get_package_share_directory('ros2_robot_description')
    xacro_file = os.path.join(pkg_description, 'urdf', 'ros2_robot.urdf.xacro')

    robot_description_substitution = Command(['xacro ', xacro_file])
    robot_description = ParameterValue(robot_description_substitution, value_type=str)
    robot_description_param = {'robot_description': robot_description}

    pkg_ros2_robot = get_package_share_directory('ros2_robot')

    rviz_config_file = os.path.join(pkg_ros2_robot, 'config', 'rviz2.rviz')
    slam_params_file = os.path.join(pkg_ros2_robot, 'config', 'slam_param.yaml')

    rsp_node = Node(
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
        default_value='97',
        description='Lidar rotation offset in degrees (180 for upside-down)'
    )

    lidar_node = Node(
        package='lidar_node',
        executable='lidar_node',
        name='lidar_node',
        parameters=[{
            'serial_port': LaunchConfiguration('serial_port'),
            'frame_id': LaunchConfiguration('frame_id'),
            'rotation': LaunchConfiguration('rotation'),
        }],
        output='screen'
    )

    odom_node = Node(
        package='ros2_robot_odom',
        executable='odom_node',
        name='odom_node',
        output='screen'
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

    time_work = 90.0 # ПОМЕНЯТЬ
    shutdown_timer = TimerAction(
        period=time_work,
        actions=[
            LogInfo(msg=f"Mission time ({time_work}s) completed. Shutting down..."),
            Shutdown(reason='Mission complete')
        ]
    )

    delayed_nodes = TimerAction(
        period=5.0,
        actions=[
            odom_node,
            slam_toolbox,
            rviz_node,
        ]
    )

    return LaunchDescription([
        rsp_node,
        joint_state_publisher_node,
        serial_port_arg,
        frame_id_arg,
        rotation_arg,
        lidar_node,
        delayed_nodes,
        shutdown_timer,
    ])
