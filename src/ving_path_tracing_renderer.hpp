#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "ving_core.hpp"
#include "ving_render_frames.hpp"

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
    };

    struct PushConstants
    {
        int sphere_count;
    };

    struct Sphere
    {
        glm::vec3 position;
        float radius;
        glm::vec4 color;
    };

  public:
    PathTracingRenderer(const Core &core, const Scene &scene);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera);

  private:
    constexpr static uint32_t sphere_count = 1;

    PushConstants m_push_constants;
    Image2D m_render_image;
    GPUBuffer m_sphere_buffer;
    std::array<Sphere, sphere_count> m_spheres;

    GPUBuffer m_camera_info_buffer;
    CameraInfo *m_camera_info;

    RenderResources m_resources;
    Pipelines m_pipelines;
};
} // namespace ving
