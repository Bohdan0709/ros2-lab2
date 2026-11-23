#include <algorithm>
#include <cctype>
#include <memory>
#include <mutex>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "lab2_interfaces/srv/reset_filter.hpp"

class DataFilterNode : public rclcpp::Node
{
public:
    DataFilterNode()
        : Node("data_filter"), processed_count_(0)
    {
        // П. 3: QoS профіль transient_local глибиною 1
        rclcpp::QoS pub_qos(1);
        pub_qos.transient_local();

        // П. 4: Перевизначення Depth, Reliability, Durability через параметри
        rclcpp::PublisherOptions pub_options;
        pub_options.qos_overriding_options = rclcpp::QosOverridingOptions({rclcpp::QosPolicyKind::Depth,
                                                                           rclcpp::QosPolicyKind::Reliability,
                                                                           rclcpp::QosPolicyKind::Durability});

        // П. 2: matched_callback
        pub_options.event_callbacks.matched_callback =
            [this](rclcpp::MatchedInfo &info)
        {
            std::lock_guard<std::mutex> lock(sub_mutex_);
            if (info.current_count > 0 && !sub_)
            {
                RCLCPP_INFO(
                    this->get_logger(),
                    "[matched_callback] З'явився слухач (%zu). Вмикаємо підписку на /input_topic!",
                    info.current_count);
                sub_ = this->create_subscription<std_msgs::msg::String>(
                    "input_topic", 10,
                    std::bind(&DataFilterNode::input_callback, this, std::placeholders::_1));
            }
            else if (info.current_count == 0 && sub_)
            {
                RCLCPP_INFO(
                    this->get_logger(),
                    "[matched_callback] Слухачів 0. Вимикаємо підписку на /input_topic (Lazy mode)!");
                sub_.reset();
            }
        };

        pub_ = this->create_publisher<std_msgs::msg::String>(
            "uppercase_topic", pub_qos, pub_options);

        // П. 5: Сервіс ResetFilter + інтроспекція RCL_SERVICE_INTROSPECTION_CONTENTS
        reset_srv_ = this->create_service<lab2_interfaces::srv::ResetFilter>(
            "reset_filter",
            std::bind(
                &DataFilterNode::handle_reset, this,
                std::placeholders::_1, std::placeholders::_2));

        reset_srv_->configure_introspection(
            this->get_clock(),
            rclcpp::ServicesQoS(),
            RCL_SERVICE_INTROSPECTION_CONTENTS);

        RCLCPP_INFO(
            this->get_logger(),
            "Вузол data_filter запущено. Очікую слухачів на /uppercase_topic...");
    }

private:
    void input_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        std::string upper = msg->data;
        std::transform(
            upper.begin(), upper.end(), upper.begin(),
            [](unsigned char c)
            { return static_cast<char>(std::toupper(c)); });

        auto out_msg = std_msgs::msg::String();
        out_msg.data = upper;
        ++processed_count_;

        RCLCPP_INFO(
            this->get_logger(),
            "Фільтр (#%zu): '%s' -> '%s'",
            processed_count_, msg->data.c_str(), out_msg.data.c_str());

        pub_->publish(out_msg);
    }

    void handle_reset(
        const std::shared_ptr<lab2_interfaces::srv::ResetFilter::Request> request,
        std::shared_ptr<lab2_interfaces::srv::ResetFilter::Response> response)
    {
        if (request->reset_stats)
        {
            processed_count_ = 0;
        }
        response->success = true;
        response->message = "Фільтр успішно скинуто! Лічильник обнулено.";
        RCLCPP_INFO(this->get_logger(), "[ResetFilter] %s", response->message.c_str());
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
    rclcpp::Service<lab2_interfaces::srv::ResetFilter>::SharedPtr reset_srv_;
    std::mutex sub_mutex_;
    size_t processed_count_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DataFilterNode>());
    rclcpp::shutdown();
    return 0;
}