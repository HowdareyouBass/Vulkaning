#pragma once

#include "ving_base_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene.hpp"

namespace ving
{
class AABBRenderer : public BaseRenderer
{
    struct PushConstants
    {
        glm::mat4 pvm_transform;
        AABB aabb;
    };

  public:
    AABBRenderer(const Core &core);

    void render_all(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene);
    void render_single(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene,
                       uint32_t id);

    std::function<void()> get_imgui();

  private:
    const Core &r_core;

    Core::Pipelines m_pipeline;

    PushConstants m_push_constants;

    // GPUBuffer m_aabb_positions_buffer;
    // std::array<glm::vec4, 8> m_aabb_positions;

    GPUBuffer m_aabb_indices;
};
} // namespace ving
