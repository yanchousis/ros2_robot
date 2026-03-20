/*
*  3iRoboticsLIDAR System II
*  Driver Interface
*
*  Copyright 2017 3iRobotics
*  All rights reserved.
*
*	Author: 3iRobotics, Data:2017-09-15
*
*/

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

#include "C3iroboticsLidar.h"
#include "../sdk/include/CSerialConnection.h"

#define DEG2RAD(x) ((x)*M_PI/180.)

typedef struct _rslidar_data
{
    _rslidar_data()
    {
        signal = 0;
        angle = 0.0;
        distance = 0.0;
    }
    uint8_t signal;
    float   angle;
    float   distance;
}RslidarDataComplete;


using namespace std;
using namespace everest::hwdrivers;

class DeltaLidarNode : public rclcpp::Node
{
public:
    DeltaLidarNode()
    : Node("delta_2b_lidar_node")
    {
        this->declare_parameter<std::string>("serial_port", "/dev/ttyUSB0");
        this->declare_parameter<std::string>("frame_id", "lidar_link");
        this->declare_parameter<int>("rotation", 180);

        this->get_parameter("serial_port", opt_com_path_);
        this->get_parameter("frame_id", frame_id_);
        this->get_parameter("rotation", rotation_);

        scan_pub_ = this->create_publisher<sensor_msgs::msg::LaserScan>("scan", 1000);

        serial_connect_.setBaud(115200);
        serial_connect_.setPort(opt_com_path_.c_str());
        if(serial_connect_.openSimple())
        {
            RCLCPP_INFO(this->get_logger(), "[AuxCtrl] Open serial port successful!");
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "[AuxCtrl] Open serial port %s failed!", opt_com_path_.c_str());
            return;
        }

        RCLCPP_INFO(this->get_logger(), "3iRoboticsLidar connected");

        robotics_lidar_.initilize(&serial_connect_);

        start_scan_time_ = this->now();
    }

    void publish_scan(_rslidar_data *nodes, size_t node_count, rclcpp::Time start,
                     double scan_time, float angle_min, float angle_max)
    {
        sensor_msgs::msg::LaserScan scan_msg;

        scan_msg.header.stamp = start;
        scan_msg.header.frame_id = frame_id_;
        scan_msg.angle_min = angle_min;
        scan_msg.angle_max = angle_max;
        scan_msg.angle_increment = (scan_msg.angle_max - scan_msg.angle_min) / (360.0f - 1.0f);

        scan_msg.scan_time = scan_time;
        scan_msg.time_increment = scan_time / (double)(node_count-1);
        scan_msg.range_min = 0.15;
        scan_msg.range_max = 5.0;

        scan_msg.ranges.resize(360, std::numeric_limits<float>::infinity());
        scan_msg.intensities.resize(360, 0.0);

        for (size_t i = 0; i < node_count; i++)
        {
            size_t current_angle = floor(nodes[i].angle);
            if(current_angle > 360.0)
            {
                printf("Lidar angle is out of range %zu\n", current_angle);
                continue;
            }
            
            int adjusted_angle = (int)current_angle + rotation_;
            if (adjusted_angle >= 360)
                adjusted_angle -= 360;
            
            float read_value = (float) nodes[i].distance;
            if (read_value < scan_msg.range_min || read_value > scan_msg.range_max)
                scan_msg.ranges[360 - 1 - adjusted_angle] = std::numeric_limits<float>::infinity();
            else
                scan_msg.ranges[360 - 1 - adjusted_angle] = read_value;

            float intensities = (float) nodes[i].signal;
            scan_msg.intensities[360 - 1 - adjusted_angle] = intensities;
        }

        scan_pub_->publish(scan_msg);
    }

    void spin()
    {
        rclcpp::Time end_scan_time;
        double scan_duration;

        while (rclcpp::ok())
        {
            TLidarGrabResult result = robotics_lidar_.getScanData();
            switch(result)
            {
                case LIDAR_GRAB_ING:
                {
                    break;
                }
                case LIDAR_GRAB_SUCESS:
                {
                    TLidarScan lidar_scan = robotics_lidar_.getLidarScan();
                    size_t lidar_scan_size = lidar_scan.getSize();
                    std::vector<RslidarDataComplete> send_lidar_scan_data;
                    send_lidar_scan_data.resize(lidar_scan_size);
                    RslidarDataComplete one_lidar_data;
                    for(size_t i = 0; i < lidar_scan_size; i++)
                    {
                        one_lidar_data.signal = lidar_scan.signal[i];
                        one_lidar_data.angle = lidar_scan.angle[i];
                        one_lidar_data.distance = lidar_scan.distance[i];
                        send_lidar_scan_data[i] = one_lidar_data;
                    }

                    float angle_min = DEG2RAD(0.0f);
                    float angle_max = DEG2RAD(359.0f);

                    end_scan_time = this->now();
                    scan_duration = (end_scan_time - start_scan_time_).seconds() * 1e-3;
                    printf("Receive Lidar count %zu!\n", lidar_scan_size);

                    publish_scan(&send_lidar_scan_data[0], lidar_scan_size,
                                 start_scan_time_, scan_duration,
                                 angle_min, angle_max);

                    start_scan_time_ = end_scan_time;

                    break;
                }
                case LIDAR_GRAB_ERRO:
                {
                    break;
                }
                case LIDAR_GRAB_ELSE:
                {
                    RCLCPP_INFO(this->get_logger(), "[Main] LIDAR_GRAB_ELSE!");
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    }

private:
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scan_pub_;
    CSerialConnection serial_connect_;
    C3iroboticsLidar robotics_lidar_;
    rclcpp::Time start_scan_time_;
    std::string opt_com_path_;
    std::string frame_id_;
    int rotation_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DeltaLidarNode>();
    node->spin();
    rclcpp::shutdown();
    return 0;
}
