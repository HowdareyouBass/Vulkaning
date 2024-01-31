#include "ving_gpu_buffer.hpp"

#include "ving_utils.hpp"

namespace ving
{

GPUBuffer::GPUBuffer(vk::Device device, vk::PhysicalDeviceMemoryProperties device_mem_props, uint32_t size,
                     vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_flags)
{
    // TODO: Abstract memory properties into device class
    auto buf_info = vk::BufferCreateInfo{}.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

    m_buffer = device.createBufferUnique(buf_info);

    auto mem_req = device.getBufferMemoryRequirements(*m_buffer);

    // TODO: Try to do custom memory allocator
    auto alloc_info =
        vk::MemoryAllocateInfo{}
            .setAllocationSize(mem_req.size)
            .setMemoryTypeIndex(utils::find_memory_type(device_mem_props, mem_req.memoryTypeBits, memory_flags));

    m_memory = device.allocateMemoryUnique(alloc_info);

    device.bindBufferMemory(*m_buffer, *m_memory, 0);
}
} // namespace ving
