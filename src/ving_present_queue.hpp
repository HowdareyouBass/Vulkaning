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
    void present_image(vk::Semaphore wait_semaphore, uint64_t frame_number);
    void copy_image_to_swapchain(vk::CommandBuffer cmd, vk::Image image, vk::Extent2D extent, uint64_t frame_number);
    void transition_swapchain_image_to_present(vk::CommandBuffer cmd, uint64_t frame_number);

  private:
    const Core &r_core;

    vktypes::Swapchain m_swapchain;
    vk::Queue m_present;
    // HACK: No need to incapsulate this
    uint32_t m_frames_in_flight;
};
} // namespace ving
