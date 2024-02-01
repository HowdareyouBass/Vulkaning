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

vk::DescriptorSet DescriptorAllocator::allocate(vk::Device device, vk::DescriptorSetLayout layout)
{
    auto alloc_info = vk::DescriptorSetAllocateInfo{}.setDescriptorPool(*m_descriptor_pool).setSetLayouts(layout);

    return device.allocateDescriptorSets(alloc_info).back();
}
void DescriptorAllocator::clear_descriptors(vk::Device device)
{
    device.resetDescriptorPool(*m_descriptor_pool);
}
} // namespace ving
