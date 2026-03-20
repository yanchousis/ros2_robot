#include <cmath>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

#define RAD2DEG(x) ((x) * 180.0 / M_PI)

class LidarClientNode : public rclcpp::Node {
 public:
  LidarClientNode() : Node("lidar_client") {
    sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
        "scan", rclcpp::SensorDataQoS(),
        std::bind(&LidarClientNode::scan_callback, this,
                  std::placeholders::_1));
  }

 private:
  void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr scan) {
    const std::size_t count = scan->ranges.size();

    for (std::size_t i = 0; i < count; ++i) {
      float degree = RAD2DEG(scan->angle_min +
                             scan->angle_increment * static_cast<float>(i));
      (void)degree;
      (void)scan->ranges[i];
      // Здесь можно обработать точки скана
      // RCLCPP_INFO(this->get_logger(), "deg=%.2f range=%.3f", degree,
      // scan->ranges[i]);
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr sub_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<LidarClientNode>());
  rclcpp::shutdown();

  return 0;
}