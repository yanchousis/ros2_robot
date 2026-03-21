#include <fcntl.h>
#include <termios.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <iostream>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <string>

class RobotNode : public rclcpp::Node {
 public:
  RobotNode() : Node("robot_node"), tf_broadcaster_(this) {
    odom_pub_ =
        this->create_publisher<nav_msgs::msg::Odometry>("/odom", 10);
    joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>(
        "/joint_states", 10);

    serial_port_ =
        this->declare_parameter<std::string>("serial_port", "/dev/ttyACM0");
    serial_baud_ = this->declare_parameter<int>("serial_baud", 115200);

    cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 10,
        std::bind(&RobotNode::cmdVelCallback, this, std::placeholders::_1));

    initSerial();

    timer_ =
        this->create_wall_timer(std::chrono::milliseconds(50),
                                std::bind(&RobotNode::parseSerial, this));
  }

  ~RobotNode() {
    if (serial_fd_ >= 0) {
      sendCommand(0.0, 0.0);
      close(serial_fd_);
    }
  }

 private:
  void initSerial() {
    serial_fd_ = open(serial_port_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_fd_ < 0) {
      RCLCPP_ERROR(this->get_logger(), "Cannot open serial port %s",
                   serial_port_.c_str());
      return;
    }

    termios tty;
    tcgetattr(serial_fd_, &tty);
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;
    tcsetattr(serial_fd_, TCSANOW, &tty);

    RCLCPP_INFO(this->get_logger(), "Serial port %s opened", serial_port_.c_str());
  }

  void sendCommand(float v, float w) {
    if (serial_fd_ < 0) return;

    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "CMD %.3f %.3f\n", v, w);
    write(serial_fd_, buffer, len);
    tcdrain(serial_fd_);
  }

  void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    float v = msg->linear.x;   // м/с
    float w = msg->angular.z;  // рад/с
    sendCommand(v, w);
  }

  void parseSerial() {
    char buf[256];
    int n = read(serial_fd_, buf, sizeof(buf) - 1);
    if (n > 0) {
      serial_buffer_ += std::string(buf, n);
      
      size_t pos;
      while ((pos = serial_buffer_.find('\n')) != std::string::npos) {
        std::string line = serial_buffer_.substr(0, pos);
        serial_buffer_ = serial_buffer_.substr(pos + 1);
        
        if (line.find("ODOM") != std::string::npos) {
          size_t odom_pos = line.find("ODOM");
          std::string data = line.substr(odom_pos + 5);
          std::istringstream iss(data);
          float x, y, theta, v_l, v_r, dt;
          if (iss >> x >> y >> theta >> v_l >> v_r >> dt) {
            publishOdometry(x, y, theta, v_l, v_r);
            publishJoints(theta);
          }
        }
      }
    }
  }

  void publishOdometry(float x, float y, float theta, float v_l, float v_r) {
    auto odom = nav_msgs::msg::Odometry();
    odom.header.stamp = this->now();
    odom.header.frame_id = "odom";
    odom.child_frame_id = "base_link";

    odom.pose.pose.position.x = x;
    odom.pose.pose.position.y = y;
    odom.pose.pose.position.z = 0.0;

    tf2::Quaternion q;
    q.setRPY(0, 0, theta);
    odom.pose.pose.orientation.x = q.x();
    odom.pose.pose.orientation.y = q.y();
    odom.pose.pose.orientation.z = q.z();
    odom.pose.pose.orientation.w = q.w();

    odom.twist.twist.linear.x = (v_l + v_r) / 2.0 / 1000.0;
    odom.twist.twist.angular.z = (v_r - v_l) / 210.0;

    odom_pub_->publish(odom);

    geometry_msgs::msg::TransformStamped tf;
    tf.header.stamp = this->now();
    tf.header.frame_id = "odom";
    tf.child_frame_id = "base_link";
    tf.transform.translation.x = x;
    tf.transform.translation.y = y;
    tf.transform.rotation = odom.pose.pose.orientation;
    tf_broadcaster_.sendTransform(tf);
  }

  void publishJoints(float theta) {
    auto joints = sensor_msgs::msg::JointState();
    joints.header.stamp = this->now();
    joints.name = {"left_wheel_joint", "right_wheel_joint"};
    joints.position = {theta, theta};
    joint_pub_->publish(joints);
  }

  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  tf2_ros::TransformBroadcaster tf_broadcaster_;
  rclcpp::TimerBase::SharedPtr timer_;

  std::string serial_port_;
  std::string serial_buffer_;
  int serial_fd_;
  int serial_baud_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RobotNode>());
  rclcpp::shutdown();

  return 0;
}
