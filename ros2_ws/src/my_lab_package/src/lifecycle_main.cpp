#include "rclcpp/rclcpp.hpp"
#include "my_lab_package/simple_lifecycle_node.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  // Створюємо ДВА окремі керовані вузли з різними іменами
  auto node_a = std::make_shared<my_lab_package::SimpleLifecycleNode>("lifecycle_node_a");
  auto node_b = std::make_shared<my_lab_package::SimpleLifecycleNode>("lifecycle_node_b");

  // Один Executor обслуговує обидва вузли одночасно
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node_a->get_node_base_interface());
  executor.add_node(node_b->get_node_base_interface());

  executor.spin();

  rclcpp::shutdown();
  return 0;
}