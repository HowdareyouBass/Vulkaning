#pragma once

#include "ving_image.hpp"
#include "ving_render_context.hpp"

namespace ving
{
class SlimeRenderer
{
  public:
    struct FrameInfo
    {
        Image2D draw_image;
    };

    SlimeRenderer(RenderContext ctx);

    FrameInfo begin_frame();
    void end_frame();
};
} // namespace ving
