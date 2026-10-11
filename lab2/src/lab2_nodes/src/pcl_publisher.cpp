#include <chrono>
#include <memory>

#include "lab2_nodes/pcl_adapter.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"

using namespace std::chrono_literals;

namespace lab2_nodes
{
using AdaptedType = rclcpp::TypeAdapter<PclCloud, sensor_msgs::msg::PointCloud2>;

class PclPublisher : public rclcpp::Node
{
public:
  explicit PclPublisher(const rclcpp::NodeOptions & options)
  // Вмикаємо внутрішньопроцесну взаємодію для цього вузла
  : Node("pcl_publisher", rclcpp::NodeOptions(options).use_intra_process_comms(true))
  {
    pub_ = create_publisher<AdaptedType>("cloud", 10);
    timer_ = create_wall_timer(1s, [this]() {
        auto cloud = std::make_unique<PclCloud>();
        cloud->header.frame_id = "map";
        for (int i = 0; i < 100; ++i) {
          cloud->push_back(pcl::PointXYZ(0.1f * i, 0.0f, 0.0f));
        }
        RCLCPP_INFO(
          get_logger(), "Публікую хмару: %zu точок, адреса %p",
          cloud->size(), static_cast<const void *>(cloud.get()));
        pub_->publish(std::move(cloud));
      });
  }

private:
  rclcpp::Publisher<AdaptedType>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace lab2_nodes

RCLCPP_COMPONENTS_REGISTER_NODE(lab2_nodes::PclPublisher)