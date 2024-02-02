#include "ving_gpu_buffer.hpp"

#include "ving_utils.hpp"

namespace ving
{

GPUBuffer::GPUBuffer(vk::Device device, vk::PhysicalDeviceMemoryProperties device_mem_props, vk::DeviceSize alloc_size,
                     vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_flags)
{
    auto buf_info =
        vk::BufferCreateInfo{}.setSize(alloc_size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

    m_buffer = device.createBufferUnique(buf_info);

    auto mem_req = device.getBufferMemoryRequirements(*m_buffer);

    // TODO: Try to do custom memory allocator
    auto alloc_info =
        vk::MemoryAllocateInfo{}
            .setAllocationSize(mem_req.size)
            .setMemoryTypeIndex(utils::find_memory_type(device_mem_props, mem_req.memoryTypeBits, memory_flags));

    m_memory = device.allocateMemoryUnique(alloc_info);

    device.bindBufferMemory(*m_buffer, *m_memory, 0);

    m_size = alloc_size;
}
void GPUBuffer::set_memory(vk::Device device, void *data, vk::DeviceSize size)
{
    void *buffer_data = device.mapMemory(*m_memory, 0, size);
    memcpy(buffer_data, data, size);
    device.unmapMemory(*m_memory);
}
void GPUBuffer::copy_to(vk::Device device, vk::Queue transfer_queue, vk::CommandPool pool, const GPUBuffer &buffer)
{
    auto alloc_info = vk::CommandBufferAllocateInfo{}.setCommandPool(pool).setCommandBufferCount(1);

    vk::UniqueCommandBuffer cmd;

    cmd = std::move(device.allocateCommandBuffersUnique(alloc_info).back());

    auto begin_info = vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd->begin(begin_info);

    auto copy_region = vk::BufferCopy{}.setSize(m_size);

    cmd->copyBuffer(*m_buffer, buffer.buffer(), copy_region);

    cmd->end();

    auto submit = vk::SubmitInfo{}.setCommandBuffers(*cmd);

    transfer_queue.submit(submit);

    transfer_queue.waitIdle();
}

} // namespace ving
