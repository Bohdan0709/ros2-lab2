#include <chrono>
#include <memory>
#include <utility>

#include "lab2_nodes/pointcloud_adapter.hpp"
#include "rclcpp_components/register_node_macro.hpp"

using namespace std::chrono_literals;

namespace lab2_ns
{

    class PointCloudTalker : public rclcpp::Node
    {
    public:
        using AdaptedType = rclcpp::TypeAdapter<SimplePointCloud, sensor_msgs::msg::PointCloud2>;

        explicit PointCloudTalker(const rclcpp::NodeOptions &options)
            : Node("pc_talker", options)
        {
            pub_ = this->create_publisher<AdaptedType>("pointcloud_topic", 10);
            timer_ = this->create_wall_timer(1s, [this]()
                                             {
      auto cloud = std::make_unique<SimplePointCloud>();
      cloud->frame_id = "lidar_link";
      cloud->points.push_back({1.0f, 2.0f, 3.0f});
      cloud->points.push_back({4.5f, 5.5f, 6.5f});

      RCLCPP_INFO(
        this->get_logger(),
        "[Talker] Публікую SimplePointCloud (%zu точок), адреса в пам'яті: %p",
        cloud->points.size(), static_cast<const void *>(cloud.get()));

      pub_->publish(std::move(cloud)); });
        }

    private:
        rclcpp::Publisher<AdaptedType>::SharedPtr pub_;
        rclcpp::TimerBase::SharedPtr timer_;
    };

} // namespace lab2_ns

RCLCPP_COMPONENTS_REGISTER_NODE(lab2_ns::PointCloudTalker)