#include <algorithm>
#include <cctype>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "lab2_interfaces/srv/reset_filter.hpp"

using ResetFilter = lab2_interfaces::srv::ResetFilter;

class DataFilter : public rclcpp::Node
{
public:
  DataFilter()
  : Node("data_filter")
  {
    rclcpp::PublisherOptions pub_options;

    pub_options.qos_overriding_options = rclcpp::QosOverridingOptions(
      {rclcpp::QosPolicyKind::Depth,
        rclcpp::QosPolicyKind::Reliability,
        rclcpp::QosPolicyKind::Durability});

    pub_options.event_callbacks.matched_callback =
      [this](auto & status) {
        on_matched(static_cast<int>(status.current_count));
      };

    auto qos = rclcpp::QoS(rclcpp::KeepLast(1)).transient_local();
    pub_ = create_publisher<std_msgs::msg::String>("uppercase_topic", qos, pub_options);

    // П.5: сервіс ResetFilter
    declare_parameter<double>("reset_delay_sec", 0.0);
    service_ = create_service<ResetFilter>(
      "reset_filter",
      [this](
        const std::shared_ptr<ResetFilter::Request>,
        std::shared_ptr<ResetFilter::Response> response) {
        on_reset(response);
      });

    // П.5: інтроспекція сервісу зі станом CONTENTS (метадані + вміст запитів/відповідей)
    service_->configure_introspection(
      get_clock(), rclcpp::SystemDefaultsQoS(), RCL_SERVICE_INTROSPECTION_CONTENTS);

    RCLCPP_INFO(get_logger(), "data_filter запущено, слухачів немає, підписку не створено");
  }

private:
  void on_matched(int listeners)
  {
    RCLCPP_INFO(get_logger(), "Кількість слухачів uppercase_topic: %d", listeners);

    if (listeners > 0 && !sub_) {
      sub_ = create_subscription<std_msgs::msg::String>(
        "input_topic", 10,
        [this](const std_msgs::msg::String & msg) {on_input(msg);});
      RCLCPP_INFO(get_logger(), "Підписку на input_topic СТВОРЕНО");
    } else if (listeners == 0 && sub_) {
      sub_.reset();
      RCLCPP_INFO(get_logger(), "Підписку на input_topic ВИМКНЕНО");
    }
  }

  void on_input(const std_msgs::msg::String & msg)
  {
    std_msgs::msg::String out;
    out.data = msg.data;
    std::transform(
      out.data.begin(), out.data.end(), out.data.begin(),
      [](unsigned char c) {return static_cast<char>(std::toupper(c));});
    pub_->publish(out);
    ++processed_count_;
  }

  void on_reset(std::shared_ptr<ResetFilter::Response> response)
  {
    // Штучна затримка (за замовчуванням 0), щоб перевірити таймаут клієнта
    double delay = get_parameter("reset_delay_sec").as_double();
    if (delay > 0.0) {
      std::this_thread::sleep_for(std::chrono::duration<double>(delay));
    }

    response->success = true;
    response->message = "Фільтр скинуто, було оброблено повідомлень: " +
      std::to_string(processed_count_);
    processed_count_ = 0;
    RCLCPP_INFO(get_logger(), "ResetFilter: %s", response->message.c_str());
  }

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
  rclcpp::Service<ResetFilter>::SharedPtr service_;
  uint64_t processed_count_{0};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DataFilter>());
  rclcpp::shutdown();
  return 0;
}