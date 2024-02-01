#pragma once

namespace ving
{
class GPUBuffer
{
  public:
    GPUBuffer() = default;
    GPUBuffer(vk::Device device, vk::PhysicalDeviceMemoryProperties device_mem_props, uint32_t alloc_size,
              vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_flags);

    void *get_mapped_memory(vk::Device device);

  private:
    vk::UniqueBuffer m_buffer;
    vk::UniqueDeviceMemory m_memory;

    vk::DeviceSize m_size;
};
} // namespace ving
