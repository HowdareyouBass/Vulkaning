#pragma once

#include "ving_render_frames.hpp"

namespace ving
{
class SimpleCubeRenderer : public BaseRenderer
{
    struct PushConstants
    {
        float dummy;
    };

  public:
    SimpleCubeRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame);

  private:
    Image2D m_depth_img;
    RenderResources m_resources;
    Pipelines m_pipelines;
};
} // namespace ving
