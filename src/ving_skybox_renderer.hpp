#pragma once

#include "ving_base_renderer.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_render_resources.hpp"
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
    };

    enum ResourceIds
    {
        Skybox
    };

  public:
    SkyboxRenderer(const Core &core);
    void render(const RenderFrames::FrameInfo &frame);

  private:
    PushConstants m_push_constants{};

    Mesh m_quad;

    RenderResources m_resources;
    Pipelines m_pipelines;
};
} // namespace ving
