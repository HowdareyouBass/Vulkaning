#pragma once

#include <glm/vec3.hpp>

#include "ving_core.hpp"
#include "ving_render_frames.hpp"

namespace ving
{
class PathTracingRenderer : public BaseRenderer
{
    enum RenderResourceIds : uint32_t
    {
        Image,
    };

    struct PushConstants
    {
        glm::vec3 dummy;
    };

  public:
    PathTracingRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame);

  private:
    Image2D m_render_image;
    RenderResources m_resources;
    Pipelines m_pipelines;
};
} // namespace ving
