#include "ving_gi_renderer.hpp"

namespace ving
{
GiRenderer::GiRenderer(const Core &core) : r_core{core}
{

    m_depth_image = core.create_image2d(vk::Extent3D{core.get_window_extent(), 1}, vk::Format::eD32Sfloat,
                                        vk::ImageUsageFlagBits::eDepthStencilAttachment);

    auto render_resource_infos = std::vector<RenderResourceCreateInfo>{
        {RenderResourceIds::Global,
         {
             {0, vk::DescriptorType::eUniformBuffer}, // Camera Info
         }},
    };
    m_resources = core.allocate_render_resources(render_resource_infos,
                                                 vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    m_cube = SceneObject{SimpleMesh::cube_interpolated_normals(core), {}};

    m_objects.push_back(SceneObject{SimpleMesh::cube_interpolated_normals(core), {}});
    m_objects.push_back(SceneObject{SimpleMesh::cube_interpolated_normals(core), {}});
    m_objects[1].transform.translation.x += 3.0f;

    // m_push_constants.vertex_buffer_address = m_cube.mesh.gpu_buffers.vertex_buffer_address;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/bin/test.vert.spv", "shaders/bin/test.frag.spv", m_resources.layouts(),
        RenderFrames::render_image_format, m_depth_image.format());
};
void GiRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene)
{
    m_camera_info = camera.camera_info();

    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);
    m_depth_image.transition_layout(cmd, vk::ImageLayout::eDepthAttachmentOptimal);

    start_rendering3d(cmd, img, m_depth_image, vk::AttachmentLoadOp::eLoad);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelines.layout.get(), 0, m_resources.descriptors(),
                           nullptr);

    for (auto &&obj : m_objects)
    {
        m_push_constants.vertex_buffer_address = obj.mesh.gpu_buffers.vertex_buffer_address;
        m_push_constants.pvm_transform = camera.projection() * camera.view() * obj.transform.mat4();

        cmd.pushConstants<PushConstants>(m_pipelines.layout.get(),
                                         vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                         m_push_constants);
        set_default_viewport_and_scissor(cmd, img);
        cmd.bindIndexBuffer(obj.mesh.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
        cmd.drawIndexed(obj.mesh.indices_count, 1, 0, 0, 0);
    }

    cmd.endRendering();
}
std::function<void()> GiRenderer::get_imgui()
{
    return []() {};
}
} // namespace ving
