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
    void map_data();
    void unmap_data();

    [[nodiscard]] void *data() const noexcept
    {
        assert(m_data != nullptr);
        return m_data;
    }
    [[nodiscard]] vk::Buffer buffer() const noexcept { return *m_buffer; }
    [[nodiscard]] vk::DeviceSize size() const noexcept { return m_size; }

    // NOTE: Can just store it in a class and get address on creation
    // WARN: Only to be used when buffer created with ShaderDeviceMemory usage
    [[nodiscard]] vk::DeviceAddress device_address() const
    {
        auto info = vk::BufferDeviceAddressInfo{m_buffer.get()};
        return m_buffer.getOwner().getBufferAddress(&info);
    }

  private:
    vk::UniqueBuffer m_buffer;
    vk::UniqueDeviceMemory m_memory;

    void *m_data{nullptr};

    vk::DeviceSize m_size;
};
} // namespace ving
