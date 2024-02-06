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

    std::vector<RenderResourceCreateInfo> bindings{
        {vk::DescriptorType::eStorageImage, 0},
        {vk::DescriptorType::eStorageBuffer, 1},
    };

    m_resources = core.allocate_render_resources(bindings, vk::ShaderStageFlagBits::eCompute);

    DescriptorWriter writer{};
    writer.write_image(0, render_target, nullptr, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
    writer.write_buffer(1, m_agents_buffer.buffer(), m_agents_buffer.size(), 0, vk::DescriptorType::eStorageBuffer);

    for (auto &&descriptor : m_resources.descriptors)
    {
        writer.update_set(core.device(), descriptor);
    }
}
void SlimeRenderer::render(const RenderFrames::FrameInfo &frame)
{
    vk::CommandBuffer cmd = frame.cmd;
    Image2D &draw_image = frame.draw_image;

    draw_image.transition_layout(cmd, vk::ImageLayout::eGeneral);
    vk::ClearColorValue clear;
    float flash = std::abs(std::sin(frame.frame_number / 120.f));
    clear = {0.0f, 0.0f, flash, 0.0f};

    auto range = def::image_subresource_range_no_mip_no_levels(vk::ImageAspectFlagBits::eColor);
    cmd.clearColorImage(draw_image.image(), draw_image.layout(), clear, range);
}
} // namespace ving
