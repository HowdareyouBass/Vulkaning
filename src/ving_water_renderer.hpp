#pragma once

#include "ving_base_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_render_resources.hpp"
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

    struct SceneData
    {
        glm::vec4 light_direction;
        glm::vec3 viewer_position;
    };

    enum ResourceIds : uint32_t
    {
        Waves,
        SceneDataId
    };

  public:
    WaterRenderer(const Core &core);

    std::function<void()> render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera);

  private:
    const Core &r_core;

    static constexpr uint32_t wave_count = 3;

    PushConstants m_push_constants;

    Image2D m_depth_img;
    RenderResources m_resources;
    SceneObject m_plane;

    std::array<Wave, wave_count> m_waves;
    GPUBuffer m_waves_buffer;

    SceneData *m_scene_data;
    GPUBuffer m_scene_data_buffer;

    Pipelines m_pipelines;
};
} // namespace ving
