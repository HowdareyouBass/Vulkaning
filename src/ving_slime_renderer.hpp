#pragma once

#include "ving_core.hpp"
#include <glm/vec2.hpp>

#include "ving_base_renderer.hpp"
#include "ving_render_frames.hpp"

namespace ving
{
class SlimeRenderer : public BaseRenderer
{
    struct Agent
    {
        glm::vec2 position;
        float angle;
        float dummy;
    };

    struct PushConstants
    {
        float delta_time;
        float time;
        float dummy;
        int agents_count;
    };

  public:
    // HARD: Temporary render_target
    SlimeRenderer(const Core &core, vk::ImageView render_traget);

    void render(const RenderFrames::FrameInfo &frame);

  private:
    static constexpr size_t agent_count = 100000;

    PushConstants m_constants{0.0f, 0.0f, 0.0f, agent_count};

    RenderResources m_resources;
    Pipelines m_pipelines;

    GPUBuffer m_agents_buffer;
    std::array<Agent, agent_count> m_agents;
};
} // namespace ving
