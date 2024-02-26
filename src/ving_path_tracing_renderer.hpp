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
        vk::DeviceAddress vertex_buffer;
        glm::vec2 dummy;
        glm::vec4 light_direction;
        int sphere_count;
    };

    struct Sphere
    {
        glm::vec3 position;
        float radius;
        glm::vec4 color;
    };

  public:
    PathTracingRenderer(const Core &core, const Scene &scene, vk::ImageView render_target);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene);
    std::function<void()> get_imgui() const;

  private:
    constexpr static uint32_t sphere_count = 2;
    constexpr static bool enable_blur = false;

    PushConstants m_push_constants;

    Mesh m_quad;

    GPUBuffer m_sphere_buffer;
    std::span<Sphere> m_spheres;

    GPUBuffer m_camera_info_buffer;
    CameraInfo *m_camera_info;

    RenderResources m_resources;
    Pipelines m_pipelines;

    Image2D m_antialiasing_image;
    RenderResources m_antialiasing_resources;
    Pipelines m_antialiasing_pipeline;
};
} // namespace ving
