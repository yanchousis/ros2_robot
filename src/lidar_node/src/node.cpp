#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "../sdk/include/CSerialConnection.h"
#include "C3iroboticsLidar.h"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

#define DEG2RAD(x) ((x) * M_PI / 180.0)

typedef struct _rslidar_data {
  _rslidar_data() {
    signal = 0;
    angle = 0.0f;
    distance = 0.0f;
  }
  uint8_t signal;
  float angle;
  float distance;
} RslidarDataComplete;

using namespace everest::hwdrivers;

class LidarDriverNode : public rclcpp::Node {
 public:
  LidarDriverNode() : Node("delta_2b_lidar_node") {
    this->declare_parameter<std::string>("serial_port", "/dev/ttyUSB4");
    this->declare_parameter<std::string>("frame_id", "laser");

    serial_port_ = this->get_parameter("serial_port").as_string();
    frame_id_ = this->get_parameter("frame_id").as_string();

    scan_pub_ = this->create_publisher<sensor_msgs::msg::LaserScan>(
        "scan", rclcpp::SensorDataQoS());

    serial_connect_.setBaud(230400);
    serial_connect_.setPort(serial_port_.c_str());

    if (!serial_connect_.openSimple()) {
      RCLCPP_ERROR(this->get_logger(), "Open serial port %s failed",
                   serial_port_.c_str());
      throw std::runtime_error("failed to open serial port");
    }

    RCLCPP_INFO(this->get_logger(), "Serial port opened: %s",
                serial_port_.c_str());
    RCLCPP_INFO(this->get_logger(), "3iRoboticsLidar connected");

    robotics_lidar_.initilize(&serial_connect_);
  }

  void run() {
    rclcpp::Time start_scan_time = this->now();

    while (rclcpp::ok()) {
      TLidarGrabResult result = robotics_lidar_.getScanData();

      switch (result) {
        case LIDAR_GRAB_ING:
          break;

        case LIDAR_GRAB_SUCESS: {
          TLidarScan lidar_scan = robotics_lidar_.getLidarScan();
          const std::size_t lidar_scan_size = lidar_scan.getSize();

          std::vector<RslidarDataComplete> send_lidar_scan_data(
              lidar_scan_size);
          for (std::size_t i = 0; i < lidar_scan_size; ++i) {
            send_lidar_scan_data[i].signal = lidar_scan.signal[i];
            send_lidar_scan_data[i].angle = lidar_scan.angle[i];
            send_lidar_scan_data[i].distance = lidar_scan.distance[i];
          }

          rclcpp::Time end_scan_time = this->now();
          const double scan_duration =
              (end_scan_time - start_scan_time).seconds();

          RCLCPP_INFO(this->get_logger(), "Receive Lidar count %zu!",
                      lidar_scan_size);

          publish_scan(send_lidar_scan_data.data(), lidar_scan_size,
                       start_scan_time, scan_duration, DEG2RAD(0.0f),
                       DEG2RAD(359.0f), frame_id_);

          start_scan_time = end_scan_time;
          break;
        }

        case LIDAR_GRAB_ERRO:
          break;

        case LIDAR_GRAB_ELSE:
          RCLCPP_WARN(this->get_logger(), "LIDAR_GRAB_ELSE");
          break;
      }

      rclcpp::sleep_for(std::chrono::microseconds(50));
    }
  }

 private:
  void publish_scan(const RslidarDataComplete* nodes, std::size_t node_count,
                    const rclcpp::Time& start, double scan_time,
                    float angle_min, float angle_max,
                    const std::string& frame_id) {
    sensor_msgs::msg::LaserScan scan_msg;

    scan_msg.header.stamp = start;
    scan_msg.header.frame_id = frame_id;

    scan_msg.angle_min = angle_min;
    scan_msg.angle_max = angle_max;
    scan_msg.angle_increment =
        (scan_msg.angle_max - scan_msg.angle_min) / (360.0f - 1.0f);

    scan_msg.scan_time = static_cast<float>(scan_time);
    scan_msg.time_increment =
        (node_count > 1) ? static_cast<float>(scan_time / (node_count - 1))
                         : 0.0f;
    scan_msg.range_min = 0.15f;
    scan_msg.range_max = 5.0f;

    scan_msg.ranges.assign(360, std::numeric_limits<float>::infinity());
    scan_msg.intensities.assign(360, 0.0f);

    for (std::size_t i = 0; i < node_count; ++i) {
      std::size_t current_angle =
          static_cast<std::size_t>(std::floor(nodes[i].angle));
      if (current_angle > 360.0) {
        RCLCPP_WARN(this->get_logger(), "Lidar angle out of range: %zu",
                    current_angle);
        continue;
      }

      float read_value = nodes[i].distance;
      const std::size_t idx = 360 - 1 - current_angle;

      if (read_value < scan_msg.range_min || read_value > scan_msg.range_max) {
        scan_msg.ranges[idx] = std::numeric_limits<float>::infinity();
      } else {
        scan_msg.ranges[idx] = read_value;
      }

      scan_msg.intensities[idx] = static_cast<float>(nodes[i].signal);
    }

    scan_pub_->publish(scan_msg);
  }

  std::string serial_port_;
  std::string frame_id_;

  rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scan_pub_;
  CSerialConnection serial_connect_;
  C3iroboticsLidar robotics_lidar_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);

  try {
    auto node = std::make_shared<LidarDriverNode>();
    node->run();
  } catch (const std::exception& e) {
    RCLCPP_FATAL(rclcpp::get_logger("delta_2b_lidar_node"), "Exception: %s",
                 e.what());
  }

  rclcpp::shutdown();
  return 0;
}