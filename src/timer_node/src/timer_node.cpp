#include <chrono>
#include <rclcpp/rclcpp.hpp>

using namespace std::chrono_literals;

class TimerNode : public rclcpp::Node {
 public:
  TimerNode() : Node("timer_node") {
    timer_ = this->create_wall_timer(
        1s, std::bind(&TimerNode::timer_callback, this));
  }

 private:
  void timer_callback() {
    RCLCPP_INFO(this->get_logger(), "1s is done");
  }
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<TimerNode>();

  try {
    rclcpp::spin(node);
  } catch (const std::exception& e) {
    RCLCPP_ERROR(node->get_logger(), "Exception: %s", e.what());
  }
  rclcpp::shutdown();

  return 0;
}
