#pragma once

#include <deque>

#include <vulkan/vulkan.hpp>

namespace ving
{
class DescriptorLayoutBuilder
{
  public:
    DescriptorLayoutBuilder &add_binding(uint32_t binding, vk::DescriptorType type);
    vk::UniqueDescriptorSetLayout build(vk::Device device, vk::ShaderStageFlags shader_stages);

  private:
    std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
};

class DescriptorAllocator
{
  public:
    struct PoolSizeRatio
    {
        vk::DescriptorType type;
        float ratio;
    };

    DescriptorAllocator() = default;
    DescriptorAllocator(vk::Device device, uint32_t max_sets, std::span<PoolSizeRatio> pool_ratios);

    std::vector<vk::DescriptorSet> allocate(vk::Device device, vk::DescriptorSetLayout layout);
    void clear_descriptors(vk::Device device);

  private:
    vk::UniqueDescriptorPool m_descriptor_pool;
};

class DescriptorWriter
{
  public:
    void write_image(int binding, vk::ImageView image, vk::Sampler sampler, vk::ImageLayout layout,
                     vk::DescriptorType type);
    void write_buffer(int binding, vk::Buffer buffer, size_t size, size_t offset, vk::DescriptorType type);
    void update_set(vk::Device device, vk::DescriptorSet set);

  private:
    std::deque<vk::DescriptorImageInfo> m_image_infos;
    std::deque<vk::DescriptorBufferInfo> m_buffer_infos;
    std::vector<vk::WriteDescriptorSet> m_writes;
};
} // namespace ving
