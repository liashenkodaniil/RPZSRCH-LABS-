#include <memory>

#include "lab2_nodes/pcl_adapter.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"

namespace lab2_nodes
{
using AdaptedType = rclcpp::TypeAdapter<PclCloud, sensor_msgs::msg::PointCloud2>;

class PclSubscriber : public rclcpp::Node
{
public:
  explicit PclSubscriber(const rclcpp::NodeOptions & options)
  : Node("pcl_subscriber", rclcpp::NodeOptions(options).use_intra_process_comms(true))
  {
    sub_ = create_subscription<AdaptedType>(
      "cloud", 10,
      [this](std::unique_ptr<PclCloud> cloud) {
        RCLCPP_INFO(
          get_logger(), "Отримано хмару: %zu точок, адреса %p",
          cloud->size(), static_cast<const void *>(cloud.get()));
      });
  }

private:
  rclcpp::Subscription<AdaptedType>::SharedPtr sub_;
};

}  // namespace lab2_nodes

RCLCPP_COMPONENTS_REGISTER_NODE(lab2_nodes::PclSubscriber)