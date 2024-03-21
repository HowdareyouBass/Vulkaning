#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene_object.hpp"

namespace ving
{
class Scene;
struct CameraInfo;
class PerspectiveCamera;

class PathTracingRenderer : public BaseRenderer
{
    enum RenderResourceIds : uint32_t
    {
        PathTracing,
        Antialiasing,
    };

    struct PushConstants
    {
        int sphere_count;
        vk::DeviceAddress vertex_buffer;
        float viewport_width;
        float viewport_height;
        uint32_t time;
    };

    struct Sphere
    {
        glm::vec3 position;
        float radius;
        glm::vec4 color;
    };
    struct SceneData
    {
        glm::vec4 light_direction;
    };

  public:
    PathTracingRenderer(const Core &core, const Scene &scene);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene);
    std::function<void()> get_imgui() const;

  private:
    constexpr static uint32_t sphere_count = 2;
    constexpr static bool enable_blur = false;

    PushConstants m_push_constants;

    Mesh m_quad;

    Image2D m_render_image;

    GPUBuffer m_sphere_buffer;
    std::span<Sphere> m_spheres;

    GPUBuffer m_camera_info_buffer;
    CameraInfo *m_camera_info;

    GPUBuffer m_scene_data_buffer;
    SceneData *m_scene_data;

    RenderResources m_resources;
    Core::Pipelines m_pipelines;

    Image2D m_antialiasing_image;
    RenderResources m_antialiasing_resources;
    Core::Pipelines m_antialiasing_pipeline;

    glm::vec3 m_previous_frame_camera_pos;
};
} // namespace ving
