#include "ving_slime_renderer.hpp"

#include <random>

#include "ving_defaults.hpp"

namespace ving
{

SlimeRenderer::SlimeRenderer(const Core &core, vk::ImageView render_target)
{
    std::default_random_engine gen;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::uniform_int_distribution dist_pos(0, static_cast<int>(core.get_window_extent().height));

    for (auto &&agent : m_agents)
    {
        agent.position = {dist_pos(gen), dist_pos(gen)};
        agent.angle = dist(gen);
    }

    m_agents_buffer = core.create_gpu_buffer(m_agents.data(), sizeof(Agent) * m_agents.size(),
                                             vk::BufferUsageFlagBits::eStorageBuffer);

    std::vector<RenderResourceCreateInfo> resources_info{
        {ResourceIds::Global, {{0, vk::DescriptorType::eStorageImage}, {1, vk::DescriptorType::eStorageBuffer}}},
    };

    m_resources = core.allocate_render_resources(resources_info, vk::ShaderStageFlagBits::eCompute);

    m_resources.get_resource(ResourceIds::Global)
        .write_image(core.device(), 0, render_target, vk::ImageLayout::eGeneral, nullptr);
    m_resources.get_resource(ResourceIds::Global).write_buffer(core.device(), 1, m_agents_buffer);

    m_pipelines =
        core.create_compute_render_pipelines<PushConstants>(m_resources.layouts(), "shaders/draw_slime.comp.spv");
}
void SlimeRenderer::render(const RenderFrames::FrameInfo &frame)
{
    vk::CommandBuffer cmd = frame.cmd;
    Image2D &draw_image = frame.draw_image;

    draw_image.transition_layout(cmd, vk::ImageLayout::eGeneral);

    // NOTE: Clear color code
    // vk::ClearColorValue clear;
    // float flash = std::abs(std::sin(frame.frame_number / 120.f));
    // clear = {0.0f, 0.0f, flash, 0.0f};
    //
    // auto range = def::image_subresource_range_no_mip_no_levels(vk::ImageAspectFlagBits::eColor);
    // cmd.clearColorImage(draw_image.image(), draw_image.layout(), clear, range);

    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_pipelines.layout.get(), 0, m_resources.descriptors(),
                           nullptr);

    m_constants.delta_time = frame.delta_time;
    m_constants.time = frame.time;

    cmd.pushConstants<PushConstants>(m_pipelines.layout.get(), vk::ShaderStageFlagBits::eCompute, 0, m_constants);

    cmd.dispatch(std::ceil(draw_image.extent().width / 16.0), std::ceil(draw_image.extent().height / 16.0), 1);
}
} // namespace ving
