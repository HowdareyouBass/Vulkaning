#pragma once

#include <glm/mat4x4.hpp>

#include "ving_render_frames.hpp"
#include "ving_render_resources.hpp"
#include "ving_scene_object.hpp"

namespace ving
{
struct PerspectiveCamera;
class SimpleCubeRenderer : public BaseRenderer
{
    struct PushConstants
    {
        glm::mat4 render_mtx{1.0f};
        vk::DeviceAddress vertex_buffer_address;
    };
    struct SceneData
    {
        glm::vec4 lightning_dir;
    };

    enum ResourceIds : uint32_t
    {
        Global
    };

  public:
    SimpleCubeRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera);

  private:
    PushConstants m_push_constants;

    SceneObject m_cube;
    SceneObject m_model;
    Image2D m_depth_img;
    RenderResources m_resources;

    SceneData m_scene_data;
    GPUBuffer m_scene_data_buffer;

    Pipelines m_pipelines;
};
} // namespace ving
