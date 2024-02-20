#include "ving_path_tracing_renderer.hpp"
#include <cmath>

namespace ving
{
PathTracingRenderer::PathTracingRenderer(const Core &core)
{
    m_render_image = core.create_image2d(vk::Extent3D{core.get_window_extent(), 1u}, vk::Format::eR8G8B8A8Unorm,
                                         vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage);

    auto resource_infos = std::vector<RenderResourceCreateInfo>{
        RenderResourceCreateInfo{RenderResourceIds::Image, {{0, vk::DescriptorType::eStorageImage}}},
    };

    m_resources = core.allocate_render_resources(resource_infos, vk::ShaderStageFlagBits::eCompute);
    m_resources.get_resource(RenderResourceIds::Image)
        .write_image(core.device(), 0, m_render_image.view(), vk::ImageLayout::eGeneral);

    m_pipelines =
        core.create_compute_render_pipelines<PushConstants>(m_resources.layouts(), "shaders/path_tracing.comp.spv");
}

void PathTracingRenderer::render(const RenderFrames::FrameInfo &frame)
{
    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;
    m_render_image.transition_layout(cmd, vk::ImageLayout::eGeneral);

    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_pipelines.layout.get(), 0, m_resources.descriptors(),
                           nullptr);
    cmd.dispatch(std::ceil(m_render_image.extent().width / 32.0), std::ceil(m_render_image.extent().height / 32.0), 1);

    m_render_image.transition_layout(cmd, vk::ImageLayout::eTransferSrcOptimal);
    img.transition_layout(cmd, vk::ImageLayout::eTransferDstOptimal);
    m_render_image.copy_to(cmd, img);
}
} // namespace ving
