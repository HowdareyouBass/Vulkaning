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

    struct CameraInfo
    {
        glm::vec3 up;
        float dummy;
        glm::vec3 right;
        float dummy1;
        glm::vec3 forward;
        float dummy2;
        glm::vec3 position;
    };

    enum ResourceIds
    {
        Skybox
    };

  public:
    SkyboxRenderer(const Core &core);
    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene);

  private:
    const Core &r_core;

    Image2D load_cube_map(std::string_view filepath);

    PushConstants m_push_constants{};

    Image2D m_skybox_cubemap;
    vk::UniqueSampler m_skybox_sampler;
    Mesh m_quad;

    CameraInfo *m_camera_info;
    GPUBuffer m_camera_info_buffer;

    RenderResources m_resources;
    Pipelines m_pipelines;
};
} // namespace ving
