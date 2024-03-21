#pragma once

#include "ving_base_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene.hpp"
#include "ving_scene_object.hpp"

namespace ving
{
class GiRenderer : public BaseRenderer
{
    struct PushConstants
    {
        glm::mat4 model_transform;
        vk::DeviceAddress vertex_buffer_address;
    };

    struct SceneData
    {
        glm::vec4 light_direction;
        uint32_t point_lights_count;
    };

    struct UniformBufferObject
    {
        CameraInfo camera_info;
        SceneData scene_data;
    };
    struct PointLight
    {
        glm::vec3 position;
        float radius;
        glm::vec4 color;
    };

    enum RenderResourceIds : uint32_t
    {
        Global,
    };

  public:
    GiRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera, const Scene &scene);
    std::function<void()> get_imgui();

  private:
    static const uint32_t point_lights_count = 1;

    const Core &r_core;

    PushConstants m_push_constants;

    Image2D m_depth_image;

    RenderResources m_resources;
    Core::Pipelines m_pipelines;

    GPUBuffer m_uniform_buffer;
    UniformBufferObject *m_ubo;

    GPUBuffer m_point_lights_buffer;
    std::span<PointLight> m_point_lights;
};
} // namespace ving
