#include "ving_gi_renderer.hpp"

#include <SDL3/SDL_log.h>

namespace ving
{
GiRenderer::GiRenderer(const Core &core) : r_core{core}
{

    m_depth_image = core.create_image2d(vk::Extent3D{core.get_window_extent(), 1}, vk::Format::eD32Sfloat,
                                        vk::ImageUsageFlagBits::eDepthStencilAttachment);

    auto render_resource_infos = std::vector<RenderResourceCreateInfo>{
        {RenderResourceIds::Global,
         {
             {0, vk::DescriptorType::eUniformBuffer}, // Camera and Scene info
             {1, vk::DescriptorType::eStorageBuffer}, // Point light buffer
         }},
    };
    m_resources = core.allocate_render_resources(render_resource_infos,
                                                 vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    m_uniform_buffer =
        core.create_cpu_visible_gpu_buffer(sizeof(UniformBufferObject), vk::BufferUsageFlagBits::eUniformBuffer);
    m_uniform_buffer.map_data();
    m_ubo = static_cast<UniformBufferObject *>(m_uniform_buffer.data());

    m_resources.get_resource(RenderResourceIds::Global).write_buffer(core.device(), 0, m_uniform_buffer);

    m_point_lights_buffer = core.create_cpu_visible_gpu_buffer(sizeof(PointLight) * point_lights_count,
                                                               vk::BufferUsageFlagBits::eStorageBuffer);
    m_point_lights_buffer.map_data();
    m_point_lights = std::span<PointLight>{static_cast<PointLight *>(m_point_lights_buffer.data()), point_lights_count};

    m_resources.get_resource(RenderResourceIds::Global).write_buffer(core.device(), 1, m_point_lights_buffer);

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/bin/lambertian_shading.vert.spv", "shaders/bin/lambertian_shading.frag.spv", m_resources.layouts(),
        RenderFrames::render_image_format, m_depth_image.format(), vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack);

    m_ubo->scene_data.point_lights_count = point_lights_count;
};
void GiRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene)
{
    m_ubo->camera_info = camera.camera_info();
    m_ubo->scene_data.light_direction = scene.light_direction;

    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);
    m_depth_image.transition_layout(cmd, vk::ImageLayout::eDepthAttachmentOptimal);

    start_rendering3d(cmd, img, m_depth_image, vk::AttachmentLoadOp::eLoad);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelines.layout.get(), 0, m_resources.descriptors(),
                           nullptr);
    set_default_viewport_and_scissor(cmd, img);

    for (auto &&obj : scene.objects)
    {
        m_push_constants.model_transform = obj.transform.mat4();
        m_push_constants.vertex_buffer_address = obj.mesh.gpu_buffers.vertex_buffer_address;

        cmd.pushConstants<PushConstants>(m_pipelines.layout.get(),
                                         vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                         m_push_constants);
        cmd.bindIndexBuffer(obj.mesh.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
        cmd.drawIndexed(obj.mesh.indices_count, 1, 0, 0, 0);
    }

    cmd.endRendering();
}
std::function<void()> GiRenderer::get_imgui()
{
    return [this]() {
        for (uint32_t i = 0; auto &&light : m_point_lights)
        {
            ImGui::Text("Light #%d:", i + 1);
            ImGui::DragFloat3(std::format("Position ##{}: ", i).data(), reinterpret_cast<float *>(&light.position),
                              0.01f);
            ImGui::DragFloat(std::format("Radius ##{}: ", i).data(), &light.radius, 0.01f, 0.05f);
            ImGui::DragFloat3(std::format("Color ##{}: ", i).data(), reinterpret_cast<float *>(&light.color), 0.01f,
                              0.0f, 1.0f);
            ++i;
        }
    };
}
} // namespace ving