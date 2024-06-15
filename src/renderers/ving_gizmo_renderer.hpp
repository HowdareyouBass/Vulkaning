#pragma once

#include "ving_base_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_gizmos.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene.hpp"

namespace ving
{
class GizmoRenderer : public BaseRenderer
{
    struct PushConstant
    {
        glm::mat4 perspective_view_transform;
        glm::vec3 object_position;
        float gizmo_length;
        int32_t highlight_gizmo_index;
    };

  public:
    GizmoRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const SceneObject &target_object,
                editor::Gizmo::Type gizmo_type, int32_t highlight_gizmo_index);

  private:
    PushConstant m_push_constants;

    Core::Pipelines m_pipelines;
};
} // namespace ving
