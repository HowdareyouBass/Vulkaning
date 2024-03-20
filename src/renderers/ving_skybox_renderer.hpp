#pragma once

#include "ving_base_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_render_resources.hpp"
#include "ving_scene.hpp"
#include "ving_scene_object.hpp"

#include <glm/mat4x4.hpp>

namespace ving
{
class SkyboxRenderer : public BaseRenderer
{
    struct PushConstants
    {
        glm::mat4 render_mtx{1.0f};
        vk::DeviceAddress vertex_buffer_address;
        glm::vec2 dummy;
        glm::vec4 light_direction;
    };

    enum ResourceIds
    {
        Skybox
    };

  public:
    SkyboxRenderer(const Core &core, const Scene &scene);
    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene);

  private:
    const Core &r_core;

    PushConstants m_push_constants{};

    Mesh m_quad;

    CameraInfo *m_camera_info;
    GPUBuffer m_camera_info_buffer;

    RenderResources m_resources;
    Pipelines m_pipelines;
};
} // namespace ving
