#include <memory>

#include "lab2_nodes/pointcloud_adapter.hpp"
#include "rclcpp_components/register_node_macro.hpp"

namespace lab2_ns
{

    class PointCloudListener : public rclcpp::Node
    {
    public:
        using AdaptedType = rclcpp::TypeAdapter<SimplePointCloud, sensor_msgs::msg::PointCloud2>;

        explicit PointCloudListener(const rclcpp::NodeOptions &options)
            : Node("pc_listener", options)
        {
            sub_ = this->create_subscription<AdaptedType>(
                "pointcloud_topic", 10,
                [this](std::unique_ptr<SimplePointCloud> msg)
                {
                    RCLCPP_INFO(
                        this->get_logger(),
                        "[Listener] Отримано SimplePointCloud (%zu точок, frame='%s'), адреса в пам'яті: %p",
                        msg->points.size(), msg->frame_id.c_str(),
                        static_cast<const void *>(msg.get()));
                });
        }

    private:
        rclcpp::Subscription<AdaptedType>::SharedPtr sub_;
    };

} // namespace lab2_ns

RCLCPP_COMPONENTS_REGISTER_NODE(lab2_ns::PointCloudListener)