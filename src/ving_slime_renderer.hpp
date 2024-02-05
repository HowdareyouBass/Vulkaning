#pragma once

#include "ving_core.hpp"
#include <glm/vec2.hpp>

#include "ving_render_frames.hpp"

namespace ving
{
class SlimeRenderer
{
    struct Agent
    {
        glm::vec2 position;
        float angle;
        float dummy;
    };

  public:
    SlimeRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame);

  private:
    static constexpr size_t agent_count = 100000;

    GPUBuffer m_agents_buffer;
    std::array<Agent, agent_count> m_agents;
};
} // namespace ving
