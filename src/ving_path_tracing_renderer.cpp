#include "ving_path_tracing_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_scene.hpp"
#include <cmath>

namespace ving
{
PathTracingRenderer::PathTracingRenderer(const Core &core, const Scene &scene)
{
    m_push_constants.sphere_count = sphere_count;

    for (auto &&sphere : m_spheres)
    {
        sphere.position = {0.0f, 0.0f, 0.0f};
        sphere.radius = 0.5f;
        sphere.color = {1.0f, 0.0f, 0.0f, 1.0f};
    }

    m_sphere_buffer = core.create_gpu_buffer(m_spheres.data(), sizeof(Sphere) * m_spheres.size(),
                                             vk::BufferUsageFlagBits::eStorageBuffer);
    m_camera_info_buffer =
        core.create_cpu_visible_gpu_buffer(sizeof(CameraInfo), vk::BufferUsageFlagBits::eUniformBuffer);
    m_camera_info_buffer.map_data();
    m_camera_info = static_cast<CameraInfo *>(m_camera_info_buffer.data());

    auto resource_infos = std::vector<RenderResourceCreateInfo>{
        RenderResourceCreateInfo{RenderResourceIds::PathTracing,
                                 {
                                     {0, vk::DescriptorType::eStorageBuffer},
                                     {1, vk::DescriptorType::eCombinedImageSampler},
                                     {2, vk::DescriptorType::eUniformBuffer},
                                 }},
    };

    m_resources = core.allocate_render_resources(resource_infos,
                                                 vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    m_resources.get_resource(RenderResourceIds::PathTracing).write_buffer(core.device(), 0, m_sphere_buffer);
    m_resources.get_resource(RenderResourceIds::PathTracing)
        .write_image(core.device(), 1, scene.skybox_cubemap, scene.skybox_sampler.get());
    m_resources.get_resource(RenderResourceIds::PathTracing).write_buffer(core.device(), 2, m_camera_info_buffer);

    m_quad = SimpleMesh::quad(core, glm::vec4{0.1f, 0.1f, 0.1f, 1.0f});
    m_push_constants.vertex_buffer = m_quad.gpu_buffers.vertex_buffer_address;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/path_tracing.vert.spv", "shaders/path_tracing.frag.spv", m_resources.layouts(),
        vk::Format::eR16G16B16A16Sfloat, vk::Format::eUndefined);
}

void PathTracingRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera,
                                 const Scene &scene)
{
    m_push_constants.light_direction = scene.light_direction;

    *m_camera_info = camera.camera_info();

    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;
    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);

    start_rendering2d(cmd, img);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelines.layout.get(), 0, m_resources.descriptors(),
                           nullptr);

    cmd.pushConstants<PushConstants>(m_pipelines.layout.get(),
                                     vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                     m_push_constants);
    auto viewport =
        vk::Viewport{}.setWidth(img.extent().width).setHeight(img.extent().height).setMinDepth(0.0f).setMaxDepth(1.0f);
    // auto viewport = vk::Viewport{}.setWidth(1000).setHeight(1000).setMinDepth(0.0f).setMaxDepth(1.0f);
    cmd.setViewport(0, viewport);
    auto scissor = vk::Rect2D{}.setOffset(vk::Offset2D{0, 0}).setExtent(img.extent());
    cmd.setScissor(0, scissor);

    cmd.bindIndexBuffer(m_quad.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    cmd.drawIndexed(m_quad.indices_count, 1, 0, 0, 0);

    cmd.endRendering();
}
} // namespace ving
