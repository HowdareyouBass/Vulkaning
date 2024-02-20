#pragma once

#include <vulkan/vulkan.hpp>

namespace ving
{
class Image2D;

class BaseRenderer
{
  public:
    struct Pipelines
    {
        vk::UniquePipeline pipeline;
        vk::UniquePipelineLayout layout;
    };

    void start_rendering2d(vk::CommandBuffer cmd, const Image2D &render_image) const;

    // virtual ~BaseRenderer();
};
} // namespace ving
