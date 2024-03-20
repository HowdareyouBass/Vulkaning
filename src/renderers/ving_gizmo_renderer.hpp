#pragma once

#include "ving_base_renderer.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene.hpp"

namespace ving
{
class GizmoRenderer
{
  public:
    GizmoRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame, const SceneObject &target_object);
};
} // namespace ving
