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
    void start_rendering3d(vk::CommandBuffer cmd, const Image2D &render_image, const Image2D &depth_image) const;
    void set_default_viewport_and_scissor(vk::CommandBuffer cmd, const Image2D &img) const;

    // virtual ~BaseRenderer();
};
} // namespace ving
