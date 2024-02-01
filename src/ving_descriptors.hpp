#pragma once

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

    vk::DescriptorSet allocate(vk::Device device, vk::DescriptorSetLayout layout);
    void clear_descriptors(vk::Device device);

  private:
    vk::UniqueDescriptorPool m_descriptor_pool;
};
} // namespace ving
