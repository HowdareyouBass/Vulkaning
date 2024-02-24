#pragma once

#include "ving_base_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene.hpp"

namespace ving
{
class GiRenderer : public BaseRenderer
{
    struct PushConstants
    {
        glm::mat4 pvm_transform;
    };

    enum RenderResourceIds : uint32_t
    {
        Global,
    };

  public:
    GiRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene);

  private:
    const Core &r_core;

    PushConstants m_push_constants;
    CameraInfo m_camera_info;

    Image2D m_depth_image;

    RenderResources m_resources;
    Pipelines m_pipelines;
};
} // namespace ving
