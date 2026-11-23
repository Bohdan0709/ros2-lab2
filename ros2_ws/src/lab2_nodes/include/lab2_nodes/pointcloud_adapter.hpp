#ifndef LAB2_NODES__POINTCLOUD_ADAPTER_HPP_
#define LAB2_NODES__POINTCLOUD_ADAPTER_HPP_

#include <cstring>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/type_adapter.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/msg/point_field.hpp"

namespace lab2_ns
{

    struct Point3D
    {
        float x;
        float y;
        float z;
    };

    struct SimplePointCloud
    {
        std::string frame_id;
        std::vector<Point3D> points;
    };

} // namespace lab2_ns

template <>
struct rclcpp::TypeAdapter<lab2_ns::SimplePointCloud, sensor_msgs::msg::PointCloud2>
{
    using is_specialized = std::true_type;
    using custom_type = lab2_ns::SimplePointCloud;
    using ros_message_type = sensor_msgs::msg::PointCloud2;

    static void convert_to_ros_message(
        const custom_type &source,
        ros_message_type &destination)
    {
        RCLCPP_WARN(
            rclcpp::get_logger("TypeAdapter"),
            ">>> ВИКЛИКАНО convert_to_ros_message()! (SimplePointCloud -> PointCloud2)");

        destination.header.frame_id = source.frame_id;
        destination.height = 1;
        destination.width = static_cast<uint32_t>(source.points.size());
        destination.is_bigendian = false;
        destination.is_dense = true;

        destination.fields.resize(3);
        const char *names[3] = {"x", "y", "z"};
        for (size_t i = 0; i < 3; ++i)
        {
            destination.fields[i].name = names[i];
            destination.fields[i].offset = static_cast<uint32_t>(i * sizeof(float));
            destination.fields[i].datatype = sensor_msgs::msg::PointField::FLOAT32;
            destination.fields[i].count = 1;
        }

        destination.point_step = sizeof(lab2_ns::Point3D);
        destination.row_step = destination.point_step * destination.width;
        destination.data.resize(destination.row_step);
        if (!source.points.empty())
        {
            std::memcpy(destination.data.data(), source.points.data(), destination.row_step);
        }
    }

    static void convert_to_custom(
        const ros_message_type &source,
        custom_type &destination)
    {
        RCLCPP_WARN(
            rclcpp::get_logger("TypeAdapter"),
            "<<< ВИКЛИКАНО convert_to_custom()! (PointCloud2 -> SimplePointCloud)");

        destination.frame_id = source.header.frame_id;
        size_t num_points = source.width * source.height;
        destination.points.resize(num_points);
        if (num_points > 0 && source.data.size() >= num_points * sizeof(lab2_ns::Point3D))
        {
            std::memcpy(
                destination.points.data(), source.data.data(),
                num_points * sizeof(lab2_ns::Point3D));
        }
    }
};

#endif // LAB2_NODES__POINTCLOUD_ADAPTER_HPP_