#include "ving_descriptors.hpp"

namespace ving
{

DescriptorLayoutBuilder &DescriptorLayoutBuilder::add_binding(uint32_t binding, vk::DescriptorType type)
{
    m_bindings.push_back(
        vk::DescriptorSetLayoutBinding{}.setBinding(binding).setDescriptorCount(1).setDescriptorType(type));

    return *this;
}
vk::UniqueDescriptorSetLayout DescriptorLayoutBuilder::build(vk::Device device, vk::ShaderStageFlags shader_stages)
{
    for (auto &&b : m_bindings)
    {
        b.stageFlags |= shader_stages;
    }

    auto info = vk::DescriptorSetLayoutCreateInfo{}.setBindings(m_bindings);

    return device.createDescriptorSetLayoutUnique(info);
}
DescriptorAllocator::DescriptorAllocator(vk::Device device, uint32_t max_sets, std::span<PoolSizeRatio> pool_ratios)
{
    std::vector<vk::DescriptorPoolSize> pool_sizes;
    for (auto &&ratio : pool_ratios)
    {
        pool_sizes.push_back(vk::DescriptorPoolSize{ratio.type, uint32_t(ratio.ratio * max_sets)});
    }

    auto pool_info = vk::DescriptorPoolCreateInfo{}.setMaxSets(max_sets).setPoolSizes(pool_sizes);

    m_descriptor_pool = device.createDescriptorPoolUnique(pool_info);
}

std::vector<vk::DescriptorSet> DescriptorAllocator::allocate(vk::Device device,
                                                             std::vector<vk::DescriptorSetLayout> layout)
{
    auto alloc_info = vk::DescriptorSetAllocateInfo{}.setDescriptorPool(*m_descriptor_pool).setSetLayouts(layout);

    return device.allocateDescriptorSets(alloc_info);
}
void DescriptorAllocator::clear_descriptors(vk::Device device)
{
    device.resetDescriptorPool(*m_descriptor_pool);
}
void DescriptorWriter::write_image(int binding, vk::ImageView image, vk::Sampler sampler, vk::ImageLayout layout,
                                   vk::DescriptorType type)
{
    vk::DescriptorImageInfo &info = m_image_infos.emplace_back(sampler, image, layout);

    auto write =
        vk::WriteDescriptorSet{}.setDstBinding(binding).setDescriptorCount(1).setDescriptorType(type).setImageInfo(
            info);

    m_writes.push_back(std::move(write));
}
void DescriptorWriter::write_buffer(int binding, vk::Buffer buffer, size_t size, size_t offset, vk::DescriptorType type)
{
    vk::DescriptorBufferInfo &info = m_buffer_infos.emplace_back(buffer, offset, size);

    auto write =
        vk::WriteDescriptorSet{}.setDstBinding(binding).setDescriptorCount(1).setDescriptorType(type).setBufferInfo(
            info);

    m_writes.push_back(std::move(write));
}
void DescriptorWriter::update_set(vk::Device device, vk::DescriptorSet set)
{
    for (auto &&write : m_writes)
    {
        write.dstSet = set;
    }

    device.updateDescriptorSets(m_writes, nullptr);
}
} // namespace ving
