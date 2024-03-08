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

    struct RayTracingPipelines
    {
        vk::UniqueHandle<vk::Pipeline, vk::DispatchLoaderDynamic> pipeline;
        vk::UniquePipelineLayout layout;
        uint32_t shader_groups_count;
    };

    void start_rendering2d(vk::CommandBuffer cmd, const Image2D &render_image,
                           vk::AttachmentLoadOp attachment_load_op = vk::AttachmentLoadOp::eClear) const;
    void start_rendering3d(vk::CommandBuffer cmd, const Image2D &render_image, const Image2D &depth_image,
                           vk::AttachmentLoadOp load_op) const;
    void set_default_viewport_and_scissor(vk::CommandBuffer cmd, const Image2D &img) const;

    // virtual ~BaseRenderer();
};
} // namespace ving
