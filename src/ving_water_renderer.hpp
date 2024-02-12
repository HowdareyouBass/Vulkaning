#pragma once

#include "ving_base_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene_object.hpp"

namespace ving
{
class WaterRenderer : public BaseRenderer
{
    struct PushConstants
    {
        glm::mat4 render_mtx{1.0f};
        vk::DeviceAddress vertex_buffer_address;
        float time;
        float delta_time;
        uint32_t wave_count;
    };

    struct Wave
    {
        float wave_length;
        float amplitude;
        float speed;
        float dummy;
        glm::vec2 direction;
    };

  public:
    WaterRenderer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera);

  private:
    static constexpr uint32_t wave_count = 10;

    PushConstants m_push_constants;

    Image2D m_depth_img;
    RenderResources m_resources;
    SceneObject m_plane;

    std::array<Wave, wave_count> m_waves;
    GPUBuffer m_waves_buffer;

    Pipelines m_pipelines;
};
} // namespace ving
