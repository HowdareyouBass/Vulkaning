#pragma once

#include <vulkan/vulkan.hpp>

namespace ving
{
class GPUBuffer
{
  public:
    GPUBuffer() = default;
    GPUBuffer(vk::Device device, vk::PhysicalDeviceMemoryProperties device_mem_props, vk::DeviceSize alloc_size,
              vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_flags);

    void set_memory(vk::Device device, void *data, vk::DeviceSize size);
    void copy_to(vk::Device device, vk::Queue transfer_queue, vk::CommandPool pool, const GPUBuffer &buffer);

    [[nodiscard]] vk::Buffer buffer() const noexcept { return *m_buffer; }
    [[nodiscard]] vk::DeviceSize size() const noexcept { return m_size; }

  private:
    vk::UniqueBuffer m_buffer;
    vk::UniqueDeviceMemory m_memory;

    vk::DeviceSize m_size;
};
} // namespace ving
