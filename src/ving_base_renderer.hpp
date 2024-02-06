#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

#include "ving_descriptors.hpp"

namespace ving
{
class BaseRenderer
{
  public:
    struct RenderResources
    {
        DescriptorAllocator allocator;
        vk::UniqueDescriptorSetLayout layout;
        std::vector<vk::DescriptorSet> descriptors;
    };
    struct Pipelines
    {
        vk::UniquePipeline pipeline;
        vk::UniquePipelineLayout layout;
    };

    struct RenderResourceCreateInfo
    {
        vk::DescriptorType type;
        uint32_t binding;
    };
    // virtual ~BaseRenderer();
};
} // namespace ving
