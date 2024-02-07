#pragma once

#include <glm/mat4x4.hpp>

#include "ving_render_frames.hpp"
#include "ving_scene_object.hpp"

namespace ving
{
class SimpleCubeRenderer : public BaseRenderer
{
    struct PushConstants
    {
        glm::mat4 render_mtx{1.0f};
        vk::DeviceAddress vertex_buffer_address;
    };

  public:
    SimpleCubeRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame);

  private:
    PushConstants m_push_constants;

    GPUMeshBuffers m_cube_mesh;
    GPUMeshBuffers m_quad_mesh;
    Image2D m_depth_img;
    RenderResources m_resources;
    Pipelines m_pipelines;
};
} // namespace ving
