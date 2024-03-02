#pragma once

#include "ving_base_renderer.hpp"
#include "ving_render_frames.hpp"

namespace ving
{
class VulkanRaytracer : public BaseRenderer
{
  public:
    VulkanRaytracer(const Core &core);

    void render(const RenderFrames::FrameInfo &frame);

  private:
    vk::UniqueAccelerationStructureKHR m_mesh_acceleration_structure;
};
} // namespace ving
