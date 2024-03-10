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
        glm::mat4 pvm_transform;
        vk::DeviceAddress vertex_buffer_address;
    };

    struct SceneData
    {
        glm::vec4 light_direction;
    };

    struct UniformBufferObject
    {
        SceneData scene_data;
        CameraInfo camera_info;
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
    const Core &r_core;

    PushConstants m_push_constants;

    Image2D m_depth_image;

    RenderResources m_resources;
    Pipelines m_pipelines;

    std::vector<SceneObject> m_objects;

    GPUBuffer m_uniform_buffer;
    UniformBufferObject *m_ubo;
};
} // namespace ving
