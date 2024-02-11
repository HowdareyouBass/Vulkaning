#include "ving_water_renderer.hpp"
#include <random>

#include <glm/gtc/constants.hpp>

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

    std::default_random_engine gen;
    std::uniform_real_distribution<float> dir_dist(0.0f, 1.0f);

    for (size_t i = 0; i < m_waves.size(); ++i)
    {
        m_waves[i].wave_length = 1.0f / glm::exp(i) * 10000.0f;
        m_waves[i].amplitude = 1.0f / glm::exp(i) * 0.005f;
        m_waves[i].direction = glm::normalize(glm::vec2{dir_dist(gen), dir_dist(gen)});
        m_waves[i].speed = 0.0001f;
    }

    m_waves[0].wave_length = 1.0f;
    m_waves[0].amplitude = 0.3f;
    m_waves[0].speed = 0.0005f;
    m_waves[0].direction = glm::vec2(0.5f, 0.5f);

    m_waves_buffer =
        core.create_gpu_buffer(m_waves.data(), m_waves.size() * sizeof(Wave), vk::BufferUsageFlagBits::eStorageBuffer);

    DescriptorWriter writer{};
    writer.write_buffer(0, m_waves_buffer.buffer(), m_waves_buffer.size(), 0, vk::DescriptorType::eStorageBuffer);
    for (auto &&descriptor : m_resources.descriptors)
    {
        writer.update_set(core.device(), descriptor);
    }

    Mesh plane = SimpleMesh::flat_plane(core, 100, 100);
    m_plane = SceneObject{std::move(plane), {}};
    m_push_constants.vertex_buffer_address = m_plane.mesh.gpu_buffers.vertex_buffer_address;
    m_push_constants.wave_count = wave_count;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/water.vert.spv", "shaders/triangle.frag.spv", m_resources.layout.get(),
        vk::Format::eR16G16B16A16Sfloat, m_depth_img.format(), vk::PolygonMode::eLine);
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
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelines.layout.get(), 0, m_resources.descriptors,
                           nullptr);
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
