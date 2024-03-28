#include "ving_aabb_renderer.hpp"

namespace ving
{

AABBRenderer::AABBRenderer(const Core &core) : r_core{core}
{
    // auto render_resource_infos = std::vector<RenderResourceCreateInfo>{
    //     {0,
    //      {
    //          {0, vk::DescriptorType::eUniformBuffer}, // AABB vertices_positions
    //      }},
    // };
    //
    // m_resources =
    //     core.allocate_render_resources({}, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    // m_aabb_positions_buffer =
    //     core.create_cpu_visible_gpu_buffer(sizeof(glm::vec4) * 8, vk::BufferUsageFlagBits::eUniformBuffer);

    // m_resources.get_resource(0).write_buffer(core.device(), 0, m_aabb_positions_buffer);

    m_pipeline = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/bin/aabb_draw.vert.spv", "shaders/bin/aabb_draw.frag.spv", {}, RenderFrames::render_image_format, {},
        vk::PolygonMode::eFill, {}, vk::PrimitiveTopology::eLineList);

    std::array<uint32_t, 24> indices{0, 1, 0, 4, 0, 3, 1, 5, 1, 2, 5, 4, 5, 6, 2, 6, 2, 3, 7, 4, 7, 6, 7, 3};

    // clang-format off
    // std::array<uint32_t, 36> indices{
    //     0, 4, 5, 0, 4, 5, 0, 4, 5, 0, 4, 5,
    //     0, 4, 5, 0, 4, 5, 0, 4, 5, 0, 4, 5,
    //     0, 4, 5, 0, 4, 5, 0, 4, 5, 0, 4, 5,
    // };
    // clang-format on

    m_aabb_indices = core.create_gpu_buffer(indices.data(), sizeof(uint32_t) * indices.size(),
                                            vk::BufferUsageFlagBits::eIndexBuffer);

    // m_aabb_positions[0] = {-0.5f, -0.5f, -0.5f, 1.0f};
    // m_aabb_positions[1] = {0.5f, -0.5f, -0.5f, 1.0f};
    // m_aabb_positions[2] = {0.5f, -0.5f, 0.5f, 1.0f};
    // m_aabb_positions[3] = {-0.5f, -0.5f, 0.5f, 1.0f};
    //
    // m_aabb_positions[4] = {-0.5f, 0.5f, -0.5f, 1.0f};
    // m_aabb_positions[5] = {0.5f, 0.5f, -0.5f, 1.0f};
    // m_aabb_positions[6] = {0.5f, 0.5f, 0.5f, 1.0f};
    // m_aabb_positions[7] = {-0.5f, 0.5f, 0.5f, 1.0f};
}

void AABBRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene)
{
    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);

    start_rendering2d(cmd, img, vk::AttachmentLoadOp::eLoad);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.pipeline.get());
    // cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline.layout.get(), 0, m_resources.descriptors(),
    //                        nullptr);
    set_default_viewport_and_scissor(cmd, img);

    // m_push_constants.pvm_transform = camera.projection() * camera.view() * scene.objects[0].transform.mat4();

    for (auto &&obj : scene.objects)
    {
        m_push_constants.pvm_transform = camera.projection() * camera.view() * obj.transform.mat4();
        m_push_constants.aabb = obj.mesh.aabb;

        // AABB aabb = obj.mesh.aabb;
        // m_aabb_positions[0] = glm::vec4{aabb.min_x, aabb.min_y, aabb.min_z, 1.0f};
        // m_aabb_positions[1] = glm::vec4{aabb.min_x, aabb.max_y, aabb.min_z, 1.0f};
        // m_aabb_positions[2] = glm::vec4{aabb.max_x, aabb.max_y, aabb.min_z, 1.0f};
        // m_aabb_positions[3] = glm::vec4{aabb.max_x, aabb.min_y, aabb.min_z, 1.0f};
        //
        // m_aabb_positions[4] = glm::vec4{aabb.min_x, aabb.min_y, aabb.max_z, 1.0f};
        // m_aabb_positions[5] = glm::vec4{aabb.min_x, aabb.max_y, aabb.max_z, 1.0f};
        // m_aabb_positions[6] = glm::vec4{aabb.max_x, aabb.max_y, aabb.max_z, 1.0f};
        // m_aabb_positions[7] = glm::vec4{aabb.max_x, aabb.min_y, aabb.max_z, 1.0f};

        // m_aabb_positions_buffer.set_memory(r_core.device(), m_aabb_positions.data(), sizeof(glm::vec4) * 8);

        cmd.pushConstants<PushConstants>(m_pipeline.layout.get(),
                                         vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                         m_push_constants);

        cmd.bindIndexBuffer(m_aabb_indices.buffer(), 0, vk::IndexType::eUint32);
        cmd.drawIndexed(m_aabb_indices.size() / sizeof(uint32_t), 1, 0, 0, 0);
    }

    cmd.endRendering();
}

std::function<void()> AABBRenderer::get_imgui()
{
    return [this]() {
        // ImGui::Spacing();
        // for (int i = 0; auto &&p : m_aabb_positions)
        // {
        //     ImGui::DragFloat3(std::format("Pos {}:", i).data(), reinterpret_cast<float *>(&p), 0.01f);
        //     ++i;
        // }
    };
}
} // namespace ving
