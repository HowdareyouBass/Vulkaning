#pragma once

#include <vulkan/vulkan.hpp>

namespace ving
{
class Image2D
{
  public:
    Image2D() = default;
    Image2D(vk::Device device, vk::PhysicalDeviceMemoryProperties device_mem_props, vk::Extent3D extent,
            vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout layout, uint32_t mip_levels_count = 1,
            uint32_t layers_count = 1, vk::ImageCreateFlags flags = {});

    void transition_layout(vk::CommandBuffer cmd, vk::ImageLayout new_layout);

    [[nodiscard]] vk::Image image() const noexcept { return m_image.get(); }
    [[nodiscard]] vk::Extent2D extent() const noexcept { return {m_extent.width, m_extent.height}; }
    [[nodiscard]] vk::ImageLayout layout() const noexcept { return m_layout; }
    [[nodiscard]] vk::ImageView view() const noexcept { return m_view.get(); }
    [[nodiscard]] vk::Format format() const noexcept { return m_format; }

  private:
    vk::UniqueImage m_image;
    vk::UniqueImageView m_view;
    vk::ImageLayout m_layout;
    vk::Extent3D m_extent;
    vk::Format m_format;

    uint32_t m_mip_count;
    uint32_t m_layer_count;

    vk::UniqueDeviceMemory m_memory;
};
} // namespace ving
