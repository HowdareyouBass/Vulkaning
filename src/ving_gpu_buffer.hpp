#pragma once

namespace ving
{
class GPUBuffer
{
  public:
    GPUBuffer() = default;
    GPUBuffer(vk::Device device, vk::PhysicalDeviceMemoryProperties device_mem_props, uint32_t size,
              vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_flags);

  private:
    vk::UniqueBuffer m_buffer;
    vk::UniqueDeviceMemory m_memory;
};
} // namespace ving
