#include "ving_render_resources.hpp"

namespace ving
{
RenderResource::RenderResource(vk::DescriptorSet descriptor, std::unordered_map<uint32_t, vk::DescriptorType> bindings)
    : m_descriptor{descriptor}, m_bindings{bindings}
{
}
void RenderResource::write_buffer(vk::Device device, uint32_t binding, const ving::GPUBuffer &buffer) const
{
    assert(m_bindings.find(binding) != m_bindings.end());

    auto info = vk::DescriptorBufferInfo{}.setBuffer(buffer.buffer()).setOffset(0).setRange(buffer.size());

    auto write = vk::WriteDescriptorSet{}
                     .setDstBinding(binding)
                     .setDescriptorCount(1)
                     .setDescriptorType(m_bindings.find(binding)->second)
                     .setBufferInfo(info)
                     .setDstSet(m_descriptor);

    device.updateDescriptorSets(write, nullptr);
}
void RenderResource::write_image(vk::Device device, uint32_t binding, const ving::Image2D &image,
                                 vk::ImageLayout layout, vk::Sampler sampler) const
{
    assert(m_bindings.find(binding) != m_bindings.end());

    auto info = vk::DescriptorImageInfo{}.setImageView(image.view()).setImageLayout(layout);

    if (sampler != nullptr)
        info.sampler = sampler;

    auto write = vk::WriteDescriptorSet{}
                     .setDstBinding(binding)
                     .setDescriptorCount(1)
                     .setDescriptorType(m_bindings.find(binding)->second)
                     .setImageInfo(info)
                     .setDstSet(m_descriptor);

    device.updateDescriptorSets(write, nullptr);
}
void RenderResource::write_image(vk::Device device, uint32_t binding, vk::ImageView image, vk::ImageLayout layout,
                                 vk::Sampler sampler) const
{
    assert(m_bindings.find(binding) != m_bindings.end());

    auto info = vk::DescriptorImageInfo{}.setImageView(image).setImageLayout(layout);

    if (sampler != nullptr)
        info.sampler = sampler;

    auto write = vk::WriteDescriptorSet{}
                     .setDstBinding(binding)
                     .setDescriptorCount(1)
                     .setDescriptorType(m_bindings.find(binding)->second)
                     .setImageInfo(info)
                     .setDstSet(m_descriptor);

    device.updateDescriptorSets(write, nullptr);
}
void RenderResource::write_acceleration_structure(vk::Device device, uint32_t binding,
                                                  vk::AccelerationStructureKHR acceleration_structure) const
{
    assert(m_bindings.find(binding) != m_bindings.end());

    auto info = vk::WriteDescriptorSetAccelerationStructureKHR{}.setAccelerationStructures(acceleration_structure);

    auto write = vk::WriteDescriptorSet{}
                     .setDstBinding(binding)
                     .setDescriptorCount(1)
                     .setDescriptorType(m_bindings.find(binding)->second)
                     .setPNext(&info)
                     .setDstSet(m_descriptor);

    device.updateDescriptorSets(write, nullptr);
}
RenderResources::RenderResources(vk::Device device, std::span<RenderResourceCreateInfo> infos,
                                 vk::ShaderStageFlags shader_stage)
    : m_device{device}
{
    m_layouts.reserve(infos.size());

    std::vector<vk::DescriptorPoolSize> sizes{};

    for (auto &&info : infos)
    {
        std::vector<vk::DescriptorSetLayoutBinding> bindings{};
        bindings.reserve(info.bindings.size());

        for (auto &&binding : info.bindings)
        {
            sizes.push_back(vk::DescriptorPoolSize{binding.second, 1});
            bindings.push_back(vk::DescriptorSetLayoutBinding{binding.first, binding.second, 1, shader_stage});
        }

        auto layout_info = vk::DescriptorSetLayoutCreateInfo{}.setBindings(bindings);

        m_layouts.push_back(device.createDescriptorSetLayout(layout_info));
    }

    auto pool_info = vk::DescriptorPoolCreateInfo{}.setMaxSets(infos.size()).setPoolSizes(sizes);

    m_pool = device.createDescriptorPool(pool_info);

    auto alloc_info = vk::DescriptorSetAllocateInfo{}.setSetLayouts(m_layouts).setDescriptorPool(m_pool);
    m_descriptors = device.allocateDescriptorSets(alloc_info);

    assert(infos.size() == m_descriptors.size());
    assert(m_descriptors.size() == m_layouts.size());

    for (size_t i = 0; i < infos.size(); ++i)
    {
        m_resources[infos[i].id] =
            RenderResource{m_descriptors[i], std::unordered_map<uint32_t, vk::DescriptorType>{infos[i].bindings.begin(),
                                                                                              infos[i].bindings.end()}};
    }
}
RenderResources::~RenderResources()
{
    for (auto &&layout : m_layouts)
    {
        if (layout != nullptr)
            m_device.destroyDescriptorSetLayout(layout);
    }
    if (m_pool != nullptr)
        m_device.destroyDescriptorPool(m_pool);
}

} // namespace ving
