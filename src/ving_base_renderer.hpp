#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

#include "ving_descriptors.hpp"

namespace ving
{
class BaseRenderer
{
  public:
    struct RenderResourcesOld
    {
        DescriptorAllocator allocator;
        std::vector<vk::UniqueDescriptorSetLayout> layouts;
        std::vector<vk::DescriptorSet> descriptors;
    };

    class RenderResources
    {
      public:
        std::vector<vk::DescriptorSet> descriptors;
        std::vector<vk::DescriptorSetLayout> layouts;

        RenderResources() = default;
        RenderResources(vk::Device device, std::vector<vk::DescriptorSet> descriptors_,
                        std::vector<vk::DescriptorSetLayout> layouts_, vk::DescriptorPool pool)
            : descriptors{descriptors_}, layouts{layouts_}, m_device{device}, m_pool{pool}
        {
        }

        // HACK: Otherwise you need to convert from Unique one to default everytime
        ~RenderResources()
        {
            for (auto &&layout : layouts)
            {
                m_device.destroyDescriptorSetLayout(layout);
            }
            m_device.destroyDescriptorPool(m_pool);
        }

      private:
        vk::Device m_device;
        vk::DescriptorPool m_pool;
    };
    struct Pipelines
    {
        vk::UniquePipeline pipeline;
        vk::UniquePipelineLayout layout;
    };

    struct RenderResourceBinding
    {
        vk::DescriptorType type;
        uint32_t binding;
    };
    struct RenderResourceCreateInfo
    {
        std::vector<RenderResourceBinding> bindings;
    };
    // virtual ~BaseRenderer();
};
} // namespace ving
