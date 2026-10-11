#ifndef LAB2_NODES__PCL_ADAPTER_HPP_
#define LAB2_NODES__PCL_ADAPTER_HPP_

#include <cstdio>
#include <type_traits>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>

#include "rclcpp/type_adapter.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"

using PclCloud = pcl::PointCloud<pcl::PointXYZ>;

template<>
struct rclcpp::TypeAdapter<PclCloud, sensor_msgs::msg::PointCloud2>
{
  using is_specialized = std::true_type;
  using custom_type = PclCloud;
  using ros_message_type = sensor_msgs::msg::PointCloud2;

  static void convert_to_ros_message(const custom_type & source, ros_message_type & destination)
  {
    std::fprintf(stderr, "[ADAPTER] convert_to_ros_message() ВИКЛИКАНО (копіювання даних)\n");
    pcl::toROSMsg(source, destination);
  }

  static void convert_to_custom(const ros_message_type & source, custom_type & destination)
  {
    std::fprintf(stderr, "[ADAPTER] convert_to_custom() ВИКЛИКАНО (копіювання даних)\n");
    pcl::fromROSMsg(source, destination);
  }
};

#endif  // LAB2_NODES__PCL_ADAPTER_HPP_