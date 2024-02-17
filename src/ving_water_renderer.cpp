#include "ving_water_renderer.hpp"
#include "ving_color.hpp"
#include <random>

#include <imgui.h>

#include <glm/gtc/constants.hpp>

namespace ving
{

WaterRenderer::WaterRenderer(const Core &core) : r_core{core}
{
    m_depth_img = core.create_image2d(vk::Extent3D{core.get_window_extent(), 1}, vk::Format::eD32Sfloat,
                                      vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eUndefined);

    auto resources_info = std::vector<RenderResourceCreateInfo>{
        RenderResourceCreateInfo{ResourceIds::Waves, {{0, vk::DescriptorType::eStorageBuffer}}},
        RenderResourceCreateInfo{ResourceIds::SceneDataId, {{0, vk::DescriptorType::eUniformBuffer}}},
    };

    m_resources = RenderResources{core.device(), resources_info,
                                  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment};

    m_waves_buffer =
        core.create_cpu_visible_gpu_buffer(wave_count * sizeof(Wave), vk::BufferUsageFlagBits::eStorageBuffer);
    m_waves_buffer.map_data();
    m_waves = std::span<Wave>{static_cast<Wave *>(m_waves_buffer.data()),
                              static_cast<Wave *>(m_waves_buffer.data()) + wave_count};

    assert(m_waves.size() == wave_count);

    // m_waves.resize(wave_count);
    generate_waves();
    // m_waves_buffer =
    // core.create_gpu_buffer(m_waves.data(), m_waves.size() * sizeof(Wave), vk::BufferUsageFlagBits::eStorageBuffer);

    // Wave *w = reinterpret_cast<Wave *>(m_waves_buffer.data());
    // std::default_random_engine gen;
    // std::uniform_real_distribution<float> dir_x_dist(-1.0f, 1.0f);
    // std::uniform_real_distribution<float> dir_y_dist(-1.0f, 1.0f);
    // w->wave_length = wave_length_coefficient;
    // w->amplitude = amplitude_coefficient;
    // w->direction = glm::normalize(glm::vec2{dir_x_dist(gen), dir_y_dist(gen)});
    // w->speed = start_speed;

    glm::vec3 light_direction = glm::normalize(glm::vec3{0.5f, 0.5f, 0.0f});

    m_scene_data_buffer =
        core.create_cpu_visible_gpu_buffer(sizeof(SceneData), vk::BufferUsageFlagBits::eUniformBuffer);
    m_scene_data_buffer.map_data();
    m_scene_data = static_cast<SceneData *>(m_scene_data_buffer.data());

    m_scene_data->light_direction = glm::vec4{light_direction, 1.0f};

    m_resources.get_resource(ResourceIds::Waves).write_buffer(core.device(), 0, m_waves_buffer);
    m_resources.get_resource(ResourceIds::SceneDataId).write_buffer(core.device(), 0, m_scene_data_buffer);

    Mesh plane = SimpleMesh::flat_plane(core, 1000, 1000, colors::slate_blue);
    m_plane = SceneObject{std::move(plane), {}};
    m_push_constants.vertex_buffer_address = m_plane.mesh.gpu_buffers.vertex_buffer_address;
    m_push_constants.wave_count = wave_count;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/water.vert.spv", "shaders/water.frag.spv", m_resources.layouts(), vk::Format::eR16G16B16A16Sfloat,
        m_depth_img.format(), vk::PolygonMode::eFill);
}

std::function<void()> WaterRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera)
{
    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    m_push_constants.time = frame.time;
    m_push_constants.delta_time = frame.delta_time;
    m_scene_data->viewer_position = camera.position;

    m_depth_img.transition_layout(cmd, vk::ImageLayout::eDepthAttachmentOptimal);
    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);

    auto clear = vk::ClearValue{}.setColor(vk::ClearColorValue{0.1f, 0.1f, 0.1f, 1.0f});

    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageView(img.view())
                                .setImageLayout(img.layout())
                                .setLoadOp(vk::AttachmentLoadOp::eLoad)
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

    // cmd.draw(m_plane.mesh.vertices_count, 1, 0, 0);
    cmd.bindIndexBuffer(m_plane.mesh.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    cmd.drawIndexed(m_plane.mesh.indices_count, 1, 0, 0, 0);

    cmd.endRendering();

    return [this]() {
        // ImGui::DragInt("Wave count", reinterpret_cast<int *>(&wave_count));
        ImGui::DragFloat("Wave Length Coefficient", &wave_length_coefficient, 0.1f, 1.0f, 200.0f);
        ImGui::DragFloat("Wave Length Power", &wave_length_power, 0.01f, 0.1f, 1.0f);
        ImGui::DragFloat("Wave Amplitude Coefficient", &amplitude_coefficient, 0.01f, 1.0f, 100.0f);
        ImGui::DragFloat("Wave Amplitude Power", &amplitude_power, 0.01f, 0.1f, 1.0f);
        ImGui::DragFloat("Start Speed", &start_speed, 0.0001f, 0.0001f, 0.1f);
        ImGui::Spacing();
        if (ImGui::Button("Generate Waves"))
        {
            generate_waves();
        }

        // for (size_t i = 0; i < m_waves.size(); ++i)
        // {
        //     ImGui::Text("Wave #%zu", i);
        //     ImGui::InputFloat("Amplitude", &m_waves[i].amplitude);
        //     ImGui::InputFloat("Wave length", &m_waves[i].wave_length);
        //     ImGui::InputFloat("Wave speed", &m_waves[i].speed);
        //     ImGui::InputFloat2("Wave direction", reinterpret_cast<float *>(&m_waves[i].direction));
        // }

        // ImGui::ShowDemoWindow();
    };
}
void WaterRenderer::generate_waves()
{
    std::default_random_engine gen;
    std::uniform_real_distribution<float> dir_x_dist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> dir_y_dist(-1.0f, 1.0f);
    // std::uniform_real_distribution<float> speed_dist(0.009f, 0.01f);

    for (size_t i = 0; i < wave_count; ++i)
    {
        m_waves[i].wave_length = glm::pow(wave_length_power, i + 1) * wave_length_coefficient;
        m_waves[i].amplitude = glm::pow(amplitude_power, i + 1) * amplitude_coefficient;
        m_waves[i].direction = glm::normalize(glm::vec2{dir_x_dist(gen), dir_y_dist(gen)});
        m_waves[i].speed = start_speed;
    }
}
} // namespace ving
