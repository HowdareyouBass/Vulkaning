#pragma once

#include "ving_core.hpp"
#include "ving_render_frames.hpp"

namespace ving
{
class PathTracingRenderer : public BaseRenderer
{
  public:
    PathTracingRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame);
};
} // namespace ving
