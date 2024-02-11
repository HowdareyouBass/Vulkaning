#pragma once

#include "ving_base_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene_object.hpp"

namespace ving
{
class WaterRenderer : public BaseRenderer
{
    struct PushConstants
    {
        glm::mat4 render_mtx{1.0f};
        vk::DeviceAddress vertex_buffer_address;
        float time;
        float delta_time;
    };

  public:
    WaterRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera);

  private:
    PushConstants m_push_constants;

    Image2D m_depth_img;
    RenderResources m_resources;
    SceneObject m_plane;

    Pipelines m_pipelines;
};
} // namespace ving