#include "ving_skybox_renderer.hpp"
#include "ving_color.hpp"

namespace ving
{
SkyboxRenderer::SkyboxRenderer(const Core &core)
{
    auto resource_infos = std::vector<RenderResourceCreateInfo>{
        RenderResourceCreateInfo{ResourceIds::Skybox,
                                 {
                                     {0, vk::DescriptorType::eCombinedImageSampler},
                                 }},
    };

    m_resources = RenderResources{core.device(), resource_infos,
                                  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment};

    // m_skybox_cubemap = core.create_image2d({}, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled,
    //                                        vk::ImageLayout::eUndefined);

    m_quad = SimpleMesh::quad(core, colors::red);
    m_push_constants.vertex_buffer_address = m_quad.gpu_buffers.vertex_buffer_address;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/skybox.vert.spv", "shaders/skybox.frag.spv", m_resources.layouts(), vk::Format::eR16G16B16A16Sfloat,
        vk::Format::eUndefined, vk::PolygonMode::eFill);
}
void SkyboxRenderer::render(const RenderFrames::FrameInfo &frame)
{
    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);

    auto clear = vk::ClearValue{}.setColor(vk::ClearColorValue{0.1f, 0.1f, 0.1f, 1.0f});

    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageView(img.view())
                                .setImageLayout(img.layout())
                                .setLoadOp(vk::AttachmentLoadOp::eClear)
                                .setClearValue(clear)
                                .setStoreOp(vk::AttachmentStoreOp::eStore);

    auto render_info = vk::RenderingInfo{}
                           .setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, img.extent()})
                           .setLayerCount(1)
                           .setColorAttachments(color_attachment);

    cmd.beginRendering(render_info);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelines.layout.get(), 0, m_resources.descriptors(),
                           nullptr);
    cmd.pushConstants<PushConstants>(m_pipelines.layout.get(),
                                     vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                     m_push_constants);

    auto viewport =
        vk::Viewport{}.setWidth(img.extent().width).setHeight(img.extent().height).setMinDepth(0.0f).setMaxDepth(1.0f);
    auto scissor = vk::Rect2D{}.setOffset(vk::Offset2D{0, 0}).setExtent(img.extent());

    cmd.setViewport(0, viewport);
    cmd.setScissor(0, scissor);

    cmd.bindIndexBuffer(m_quad.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    cmd.drawIndexed(m_quad.indices_count, 1, 0, 0, 0);

    cmd.endRendering();
}
} // namespace ving
