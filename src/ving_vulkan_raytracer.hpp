#pragma once

#include "ving_base_renderer.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene_object.hpp"

namespace ving
{
class VulkanRaytracer : public BaseRenderer
{
  public:
    VulkanRaytracer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame);

  private:
    RayTracedMesh m_cube;
    GPUBuffer m_bottom_accs_buffer;

    vk::DispatchLoaderDynamic m_dispatch;

    vk::UniqueHandle<vk::AccelerationStructureKHR, vk::DispatchLoaderDynamic> m_bottom_accs;
};
} // namespace ving
