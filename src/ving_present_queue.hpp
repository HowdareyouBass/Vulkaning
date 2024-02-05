#pragma once

#include "ving_core.hpp"
#include "vk_types.hpp"

namespace ving
{
class PresentQueue
{
  public:
    PresentQueue(const Core &core, uint32_t frames_in_flight_count);

    uint32_t acquire_image(vk::Semaphore image_acquire_semaphore);
    void present_image(vk::Semaphore wait_semaphore, uint32_t image_index);
    void transition_swapchain_to_present(vk::CommandBuffer cmd, uint32_t index);

  private:
    const Core &r_core;

    vktypes::Swapchain m_swapchain;
    vk::Queue m_present;
};
} // namespace ving
