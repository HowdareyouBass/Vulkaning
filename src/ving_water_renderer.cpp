#include "ving_water_renderer.hpp"

namespace ving
{

WaterRenderer::WaterRenderer(const Core &core)
{
    m_depth_img = core.create_image2d(vk::Extent3D{core.get_window_extent(), 1}, vk::Format::eD32Sfloat,
                                      vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eUndefined);
    auto resource_info = std::vector<RenderResourceCreateInfo>{
        RenderResourceCreateInfo{vk::DescriptorType::eStorageBuffer, 0},
    };
    m_resources = core.allocate_render_resources(resource_info,
                                                 vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    Mesh plane = SimpleMesh::flat_plane(core, 100, 100);
    m_plane = SceneObject{std::move(plane), {}};
    m_push_constants.vertex_buffer_address = m_plane.mesh.gpu_buffers.vertex_buffer_address;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/water.vert.spv", "shaders/triangle.frag.spv", m_resources.layout.get(),
        vk::Format::eR16G16B16A16Sfloat, m_depth_img.format(), vk::PolygonMode::eFill);
}

void WaterRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera)
{
    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;
    m_push_constants.time = frame.time;
    m_push_constants.delta_time = frame.delta_time;

    m_depth_img.transition_layout(cmd, vk::ImageLayout::eDepthAttachmentOptimal);
    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);

    auto clear = vk::ClearValue{}.setColor(vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f});

    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageView(img.view())
                                .setImageLayout(img.layout())
                                .setLoadOp(vk::AttachmentLoadOp::eClear)
                                .setClearValue(clear)
                                .setStoreOp(vk::AttachmentStoreOp::eStore);

    auto depth_attachment =
        vk::RenderingAttachmentInfo{}
            .setImageView(m_depth_img.view())
            .setImageLayout(m_depth_img.layout())
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue{}.setDepthStencil(vk::ClearDepthStencilValue{}.setDepth(0.0f)));

    auto render_info = vk::RenderingInfo{}
                           .setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, img.extent()})
                           .setLayerCount(1)
                           .setColorAttachments(color_attachment)
                           .setPDepthAttachment(&depth_attachment);

    auto projection_view = camera.projection() * camera.view();
    m_push_constants.render_mtx = projection_view * m_plane.transform.mat4();

    cmd.beginRendering(render_info);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.pipeline.get());
    // cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelines.layout.get(), 0, m_resources.descriptors,
    //                        nullptr);
    cmd.pushConstants<PushConstants>(m_pipelines.layout.get(),
                                     vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                     m_push_constants);
    auto viewport =
        vk::Viewport{}.setWidth(img.extent().width).setHeight(img.extent().height).setMinDepth(0.0f).setMaxDepth(1.0f);
    auto scissor = vk::Rect2D{}.setOffset(vk::Offset2D{0, 0}).setExtent(img.extent());
    cmd.setViewport(0, viewport);
    cmd.setScissor(0, scissor);

    // cmd.draw(m_plane.mesh.vertices_count, 1, 0, 0);
    cmd.bindIndexBuffer(m_plane.mesh.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    cmd.drawIndexed(m_plane.mesh.indices_count, 1, 0, 0, 0);

    cmd.endRendering();
}
} // namespace ving
