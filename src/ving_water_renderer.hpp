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

    // TODO: To change wave_count need to reallocate gpu buffer
    uint32_t wave_count = 25;
    float wave_length_power = 0.85f;
    float amplitude_power = 0.73f;
    float wave_length_coefficient = 13.3f;
    float amplitude_coefficient = 2.27f;
    float start_speed = 0.007f;

    std::function<void()> render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera);

  private:
    void generate_waves();

    const Core &r_core;

    PushConstants m_push_constants;

    Image2D m_depth_img;
    RenderResources m_resources;
    SceneObject m_plane;

    // std::vector<Wave> m_waves;
    std::span<Wave> m_waves;
    GPUBuffer m_waves_buffer;

    SceneData *m_scene_data;
    GPUBuffer m_scene_data_buffer;

    Pipelines m_pipelines;
};
} // namespace ving
