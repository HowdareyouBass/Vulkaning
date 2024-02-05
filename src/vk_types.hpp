#pragma once

#include <vulkan/vulkan.hpp>

namespace ving
{
namespace vktypes
{
struct Swapchain
{
    vk::UniqueSwapchainKHR swapchain;
    vk::Format image_format;
    vk::Extent2D image_extent;
    vk::ImageLayout image_layout;
    std::vector<vk::Image> images;
    std::vector<vk::UniqueImageView> image_views;
};

} // namespace vktypes
} // namespace ving
