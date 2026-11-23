#include <chrono>
#include <future>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "lab2_interfaces/srv/reset_filter.hpp"

using namespace std::chrono_literals;

class ResetClientNode : public rclcpp::Node
{
public:
    ResetClientNode()
        : Node("reset_client")
    {
        timer_cb_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
        client_cb_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

        client_ = this->create_client<lab2_interfaces::srv::ResetFilter>(
            "reset_filter", rclcpp::ServicesQoS(), client_cb_group_);

        client_->configure_introspection(
            this->get_clock(),
            rclcpp::ServicesQoS(),
            RCL_SERVICE_INTROSPECTION_CONTENTS);

        timer_ = this->create_wall_timer(
            5s, std::bind(&ResetClientNode::on_timer, this), timer_cb_group_);

        RCLCPP_INFO(this->get_logger(), "Клієнт reset_client запущено (виклик кожні 5 с).");
    }

private:
    void on_timer()
    {
        if (!client_->wait_for_service(1s))
        {
            RCLCPP_WARN(this->get_logger(), "Сервіс /reset_filter недоступний!");
            return;
        }

        auto request = std::make_shared<lab2_interfaces::srv::ResetFilter::Request>();
        request->reset_stats = true;

        RCLCPP_INFO(this->get_logger(), "Надсилаю асинхронний запит ResetFilter...");
        auto future = client_->async_send_request(request);

        auto status = future.wait_for(2s);
        if (status == std::future_status::ready)
        {
            auto response = future.get();
            RCLCPP_INFO(
                this->get_logger(),
                "Відповідь сервісу: success=%s, message='%s'",
                response->success ? "true" : "false",
                response->message.c_str());
        }
        else if (status == std::future_status::timeout)
        {
            RCLCPP_ERROR(this->get_logger(), "Таймаут очікування відповіді (2с)!");
        }
        else
        {
            RCLCPP_ERROR(this->get_logger(), "Помилка очікування future!");
        }
    }

    rclcpp::CallbackGroup::SharedPtr timer_cb_group_;
    rclcpp::CallbackGroup::SharedPtr client_cb_group_;
    rclcpp::Client<lab2_interfaces::srv::ResetFilter>::SharedPtr client_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ResetClientNode>();
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}