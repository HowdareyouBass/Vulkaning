#pragma once

#include "ving_base_renderer.hpp"
#include "ving_camera.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene_object.hpp"

namespace ving
{
class VulkanRaytracer : public BaseRenderer
{
    struct PushConstants
    {
        glm::vec4 light;
    };

    struct Ubo
    {
        glm::mat4 view_inverse;
        glm::mat4 proj_inverse;
    };

    enum RenderResourceIds : uint32_t
    {
        Main,
    };

  public:
    VulkanRaytracer(const Core &core, RenderFrames &render_frames);

    void render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera);

  private:
    const Core &r_core;

    RayTracedMesh m_cube;

    vk::DispatchLoaderDynamic m_dispatch;

    Image2D m_render_image;
    Ubo *m_ubo;
    GPUBuffer m_ubo_buffer;

    vk::UniqueHandle<vk::AccelerationStructureKHR, vk::DispatchLoaderDynamic> m_bottom_accs;
    GPUBuffer m_bottom_accs_buffer;
    vk::DeviceAddress m_bottom_accs_address;

    vk::UniqueHandle<vk::AccelerationStructureKHR, vk::DispatchLoaderDynamic> m_top_accs;
    GPUBuffer m_top_accs_buffer;
    vk::DeviceAddress m_top_accs_address;

    GPUBuffer m_instances_buffer;

    RenderResources m_resources;
    Core::RayTracingPipelines m_pipelines;

    vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_raytracing_pipeline_properties;
    vk::PhysicalDeviceAccelerationStructureFeaturesKHR m_acceleration_structure_features;

    GPUBuffer m_raygen_sbt_buffer;
    GPUBuffer m_miss_sbt_buffer;
    GPUBuffer m_hit_sbt_buffer;

    void create_binding_table();
};
} // namespace ving
