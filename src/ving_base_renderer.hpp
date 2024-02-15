#pragma once

#include <vulkan/vulkan.hpp>

namespace ving
{
class BaseRenderer
{
  public:
    struct Pipelines
    {
        vk::UniquePipeline pipeline;
        vk::UniquePipelineLayout layout;
    };

    // virtual ~BaseRenderer();
};
} // namespace ving
