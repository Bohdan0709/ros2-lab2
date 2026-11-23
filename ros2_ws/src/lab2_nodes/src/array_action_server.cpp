#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "lab2_interfaces/action/process_array.hpp"

class ArrayActionServer : public rclcpp::Node
{
public:
    using ProcessArray = lab2_interfaces::action::ProcessArray;
    using GoalHandle = rclcpp_action::ServerGoalHandle<ProcessArray>;

    ArrayActionServer()
        : Node("array_action_server")
    {
        using namespace std::placeholders;
        this->action_server_ = rclcpp_action::create_server<ProcessArray>(
            this,
            "process_array",
            std::bind(&ArrayActionServer::handle_goal, this, _1, _2),
            std::bind(&ArrayActionServer::handle_cancel, this, _1),
            std::bind(&ArrayActionServer::handle_accepted, this, _1));

        RCLCPP_INFO(this->get_logger(), "Action-сервер /process_array готовий до роботи!");
    }

private:
    rclcpp_action::Server<ProcessArray>::SharedPtr action_server_;

    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID &,
        std::shared_ptr<const ProcessArray::Goal> goal)
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Отримано нову ціль: масив із %zu елементів",
            goal->input_data.size());
        if (goal->input_data.empty())
        {
            RCLCPP_WARN(this->get_logger(), "Порожній масив відхилено!");
            return rclcpp_action::GoalResponse::REJECT;
        }
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandle>)
    {
        RCLCPP_INFO(this->get_logger(), "Отримано запит на скасування цілі (Cancel).");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle)
    {
        std::thread{std::bind(&ArrayActionServer::execute, this, std::placeholders::_1), goal_handle}.detach();
    }

    void execute(const std::shared_ptr<GoalHandle> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Починаю обробку масиву в окремому потоці...");
        rclcpp::Rate loop_rate(2); // 2 Гц (0.5 с на елемент)

        const auto goal = goal_handle->get_goal();
        auto feedback = std::make_shared<ProcessArray::Feedback>();
        auto result = std::make_shared<ProcessArray::Result>();

        const size_t total = goal->input_data.size();
        result->output_data.reserve(total);

        for (size_t i = 0; i < total && rclcpp::ok(); ++i)
        {
            if (goal_handle->is_canceling())
            {
                result->total_items = static_cast<int32_t>(result->output_data.size());
                goal_handle->canceled(result);
                RCLCPP_INFO(this->get_logger(), "Ціль скасовано (Canceled)!");
                return;
            }

            int32_t processed_val = goal->input_data[i] * 2;
            result->output_data.push_back(processed_val);

            feedback->progress_percent = static_cast<float>(i + 1) / static_cast<float>(total) * 100.0f;
            feedback->current_status = "Оброблено елемент " + std::to_string(i + 1) +
                                       " з " + std::to_string(total);
            goal_handle->publish_feedback(feedback);
            RCLCPP_INFO(this->get_logger(), "Feedback: %.1f%% (%s)",
                        feedback->progress_percent, feedback->current_status.c_str());

            loop_rate.sleep();
        }

        if (rclcpp::ok())
        {
            result->total_items = static_cast<int32_t>(result->output_data.size());
            goal_handle->succeed(result);
            RCLCPP_INFO(this->get_logger(), "Обробку завершено успішно (Стан: Succeeded)!");
        }
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ArrayActionServer>());
    rclcpp::shutdown();
    return 0;
}