#include <imgui.h>

#include "ving_camera.hpp"
#include "ving_imgui_renderer.hpp"
#include "ving_path_tracing_renderer.hpp"
#include "ving_scene.hpp"

#include <SDL3/SDL_log.h>

namespace ving
{
PathTracingRenderer::PathTracingRenderer(const Core &core, const Scene &scene)
{
    m_push_constants.sphere_count = sphere_count;

    // for (auto &&sphere : m_spheres)
    // {
    //     sphere.position = {1.0f, 0.0f, 0.0f};
    //     sphere.radius = 0.5f;
    //     sphere.color = {1.0f, 0.0f, 0.0f, 1.0f};
    // }
    m_render_image =
        core.create_image2d(vk::Extent3D{core.get_window_extent(), 1}, vk::Format::eR16G16B16A16Sfloat,
                            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst |
                                vk::ImageUsageFlagBits::eTransferSrc);

    m_sphere_buffer =
        core.create_cpu_visible_gpu_buffer(sizeof(Sphere) * sphere_count, vk::BufferUsageFlagBits::eStorageBuffer);
    m_sphere_buffer.map_data();
    m_spheres = std::span<Sphere>{static_cast<Sphere *>(m_sphere_buffer.data()),
                                  static_cast<Sphere *>(m_sphere_buffer.data()) + sphere_count};

    m_spheres[0].radius = 100.0f;
    m_spheres[0].color = {0.0f, 1.0f, 0.0f, 1.0f};
    m_spheres[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
    m_spheres[0].position = {-2.0f, -100.0f, 10.0f};

    m_spheres[1].radius = 1.3f;
    m_spheres[1].color = {0.0f, 0.0f, 1.0f, 1.0f};
    m_spheres[1].position = {4.0f, 3.0f, 10.0f};

    m_camera_info_buffer =
        core.create_cpu_visible_gpu_buffer(sizeof(CameraInfo), vk::BufferUsageFlagBits::eUniformBuffer);
    m_camera_info_buffer.map_data();
    m_camera_info = static_cast<CameraInfo *>(m_camera_info_buffer.data());

    m_scene_data_buffer =
        core.create_cpu_visible_gpu_buffer(sizeof(SceneData), vk::BufferUsageFlagBits::eUniformBuffer);
    m_scene_data_buffer.map_data();
    m_scene_data = static_cast<SceneData *>(m_scene_data_buffer.data());

    auto resource_infos = std::vector<RenderResourceCreateInfo>{
        RenderResourceCreateInfo{RenderResourceIds::PathTracing,
                                 {
                                     {0, vk::DescriptorType::eStorageBuffer},
                                     {1, vk::DescriptorType::eCombinedImageSampler},
                                     {2, vk::DescriptorType::eUniformBuffer},
                                     {3, vk::DescriptorType::eUniformBuffer},
                                 }},
    };

    m_resources = core.allocate_render_resources(resource_infos,
                                                 vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    m_resources.get_resource(RenderResourceIds::PathTracing).write_buffer(core.device(), 0, m_sphere_buffer);
    m_resources.get_resource(RenderResourceIds::PathTracing)
        .write_image(core.device(), 1, scene.skybox_cubemap, vk::ImageLayout::eShaderReadOnlyOptimal,
                     scene.skybox_cubemap.sampler());
    m_resources.get_resource(RenderResourceIds::PathTracing).write_buffer(core.device(), 2, m_camera_info_buffer);
    m_resources.get_resource(RenderResourceIds::PathTracing).write_buffer(core.device(), 3, m_scene_data_buffer);

    m_quad = Mesh::quad(core, glm::vec4{0.1f, 0.1f, 0.1f, 1.0f});
    m_push_constants.vertex_buffer = m_quad.gpu_buffers.vertex_buffer_address;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/bin/path_tracing.vert.spv", "shaders/bin/path_tracing.frag.spv", m_resources.layouts(),
        vk::Format::eR16G16B16A16Sfloat, vk::Format::eUndefined);

    // Blur
    if constexpr (enable_blur)
    {
        auto antialiasing_resource_infos = std::vector<RenderResourceCreateInfo>{
            RenderResourceCreateInfo{
                RenderResourceIds::Antialiasing,
                {
                    {0, vk::DescriptorType::eStorageImage},
                    {1, vk::DescriptorType::eStorageImage},
                },
            },
        };
        m_antialiasing_image =
            core.create_image2d(vk::Extent3D{core.get_window_extent(), 1}, vk::Format::eR16G16B16A16Sfloat,
                                vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc);
        m_antialiasing_resources =
            core.allocate_render_resources(antialiasing_resource_infos, vk::ShaderStageFlagBits::eCompute);

        m_antialiasing_resources.get_resource(RenderResourceIds::Antialiasing)
            .write_image(core.device(), 0, m_render_image, vk::ImageLayout::eGeneral);
        m_antialiasing_resources.get_resource(RenderResourceIds::Antialiasing)
            .write_image(core.device(), 1, m_antialiasing_image, vk::ImageLayout::eGeneral);

        m_antialiasing_pipeline = core.create_compute_render_pipelines<PushConstants>(
            m_antialiasing_resources.layouts(), "shaders/bin/antialiasing.comp.spv");
    }
}

void PathTracingRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera,
                                 const Scene &scene)
{
    m_scene_data->light_direction = scene.light_direction;
    *m_camera_info = camera.camera_info();

    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    m_render_image.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);

    auto load_op = vk::AttachmentLoadOp::eLoad;

    if (m_previous_frame_camera_pos != camera.position)
    {
        load_op = vk::AttachmentLoadOp::eClear;
    }
    start_rendering2d(cmd, m_render_image, load_op);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelines.layout.get(), 0, m_resources.descriptors(),
                           nullptr);

    m_push_constants.viewport_width = m_render_image.extent().width;
    m_push_constants.viewport_height = m_render_image.extent().height;
    m_push_constants.time = frame.time;

    cmd.pushConstants<PushConstants>(m_pipelines.layout.get(),
                                     vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                     m_push_constants);

    set_default_viewport_and_scissor(cmd, m_render_image);

    cmd.bindIndexBuffer(m_quad.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    cmd.drawIndexed(m_quad.indices_count, 1, 0, 0, 0);

    cmd.endRendering();

    if constexpr (enable_blur)
    {
        m_render_image.transition_layout(cmd, vk::ImageLayout::eGeneral);
        m_antialiasing_image.transition_layout(cmd, vk::ImageLayout::eGeneral);

        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, m_antialiasing_pipeline.pipeline.get());
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_antialiasing_pipeline.layout.get(), 0,
                               m_antialiasing_resources.descriptors(), nullptr);
        cmd.pushConstants<PushConstants>(m_antialiasing_pipeline.layout.get(), vk::ShaderStageFlagBits::eCompute, 0,
                                         m_push_constants);

        cmd.dispatch(std::ceil(m_render_image.extent().width / 16.0), std::ceil(m_render_image.extent().height / 16.0),
                     1);

        m_render_image.transition_layout(cmd, vk::ImageLayout::eTransferDstOptimal);
        m_antialiasing_image.transition_layout(cmd, vk::ImageLayout::eTransferSrcOptimal);
        m_antialiasing_image.copy_to(cmd, m_render_image);
    }

    m_render_image.transition_layout(cmd, vk::ImageLayout::eTransferSrcOptimal);
    img.transition_layout(cmd, vk::ImageLayout::eTransferDstOptimal);
    m_render_image.copy_to(cmd, img);

    m_previous_frame_camera_pos = camera.position;
}

std::function<void()> PathTracingRenderer::get_imgui() const
{
    return [this]() {
        for (size_t i = 0; i < m_spheres.size(); ++i)
        {
            ImGui::Text("%s", std::format("Sphere #{}", i).c_str());
            ImGui::DragFloat3(std::format("Position ##{}", i).c_str(),
                              reinterpret_cast<float *>(&m_spheres[i].position), 0.01f);
            ImGui::DragFloat(std::format("Radius ##{}", i).c_str(), &m_spheres[i].radius, 0.005f);
            ImGui::ColorEdit4(std::format("Color ##{}", i).c_str(), reinterpret_cast<float *>(&m_spheres[i].color));
        }
    };
}

} // namespace ving
