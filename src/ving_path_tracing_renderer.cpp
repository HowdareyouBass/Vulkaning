#include "ving_path_tracing_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_scene.hpp"
#include <cmath>

namespace ving
{
PathTracingRenderer::PathTracingRenderer(const Core &core, const Scene &scene)
{
    m_push_constants.sphere_count = sphere_count;

    m_render_image = core.create_image2d(vk::Extent3D{core.get_window_extent(), 1u}, vk::Format::eR8G8B8A8Unorm,
                                         vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage);

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
                                     {0, vk::DescriptorType::eStorageImage},
                                     {1, vk::DescriptorType::eStorageBuffer},
                                     {2, vk::DescriptorType::eCombinedImageSampler},
                                     {3, vk::DescriptorType::eUniformBuffer},
                                 }},
    };

    m_resources = core.allocate_render_resources(resource_infos, vk::ShaderStageFlagBits::eCompute);
    m_resources.get_resource(RenderResourceIds::PathTracing)
        .write_image(core.device(), 0, m_render_image.view(), vk::ImageLayout::eGeneral);
    m_resources.get_resource(RenderResourceIds::PathTracing).write_buffer(core.device(), 1, m_sphere_buffer);
    m_resources.get_resource(RenderResourceIds::PathTracing)
        .write_image(core.device(), 2, scene.skybox_cubemap, scene.skybox_sampler.get());
    m_resources.get_resource(RenderResourceIds::PathTracing).write_buffer(core.device(), 3, m_camera_info_buffer);

    m_pipelines =
        core.create_compute_render_pipelines<PushConstants>(m_resources.layouts(), "shaders/path_tracing.comp.spv");
}

void PathTracingRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera)
{
    *m_camera_info = camera.camera_info();

    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;
    m_render_image.transition_layout(cmd, vk::ImageLayout::eGeneral);

    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_pipelines.layout.get(), 0, m_resources.descriptors(),
                           nullptr);
    cmd.pushConstants<PushConstants>(m_pipelines.layout.get(), vk::ShaderStageFlagBits::eCompute, 0, m_push_constants);
    cmd.dispatch(std::ceil(m_render_image.extent().width / 32.0), std::ceil(m_render_image.extent().height / 32.0), 1);

    m_render_image.transition_layout(cmd, vk::ImageLayout::eTransferSrcOptimal);
    img.transition_layout(cmd, vk::ImageLayout::eTransferDstOptimal);
    m_render_image.copy_to(cmd, img);
}
} // namespace ving
