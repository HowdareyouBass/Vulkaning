#pragma once

namespace ving
{
class Image2D
{
  public:
    Image2D() = default;
    Image2D(vk::Device device, vk::PhysicalDeviceMemoryProperties device_mem_props, vk::Extent3D extent,
            vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout layout);

    void transition_layout(vk::CommandBuffer cmd, vk::ImageLayout new_layout);

    [[nodiscard]] vk::Image image() const noexcept { return *m_image; }
    [[nodiscard]] vk::Extent2D extent() const noexcept { return {m_extent.width, m_extent.height}; }
    [[nodiscard]] vk::ImageLayout layout() const noexcept { return m_layout; }
    [[nodiscard]] vk::ImageView view() const noexcept { return *m_view; }

  private:
    vk::UniqueImage m_image;
    vk::UniqueImageView m_view;
    vk::ImageLayout m_layout;
    vk::Extent3D m_extent;
    vk::Format m_format;

    vk::UniqueDeviceMemory m_memory;
};
} // namespace ving
