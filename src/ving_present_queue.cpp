#include "ving_present_queue.hpp"
#include "ving_utils.hpp"

namespace ving
{
PresentQueue::PresentQueue(const Core &core, uint32_t frames_in_flight_count)
    : r_core{core}, m_frames_in_flight{frames_in_flight_count}
{
    m_swapchain = core.create_swapchain(frames_in_flight_count);
    m_present = core.get_present_queue();
}

uint32_t PresentQueue::acquire_image(vk::Semaphore image_acquire_semaphore)
{
    return r_core.acquire_swapchain_image(*m_swapchain.swapchain, image_acquire_semaphore);
}

void PresentQueue::present_image(vk::Semaphore wait_semaphore, uint64_t frame_number)
{
    uint32_t frame_index = frame_number % m_frames_in_flight;

    auto present_info = vk::PresentInfoKHR{}
                            .setSwapchains(m_swapchain.swapchain.get())
                            .setWaitSemaphores(wait_semaphore)
                            .setImageIndices(frame_index);

    vk::resultCheck(m_present.presentKHR(&present_info), "Present image failed");
}

// FIXME: These functions are called one after another only
void PresentQueue::copy_image_to_swapchain(vk::CommandBuffer cmd, vk::Image image, vk::Extent2D extent,
                                           uint64_t frame_number)
{
    utils::transition_image(cmd, m_swapchain.images[frame_number % m_frames_in_flight], vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eTransferDstOptimal);
    utils::copy_image_to_image(cmd, image, m_swapchain.images[frame_number % m_frames_in_flight], extent,
                               m_swapchain.image_extent);
}
void PresentQueue::transition_swapchain_image_to_present(vk::CommandBuffer cmd, uint64_t frame_number)
{
    utils::transition_image(cmd, m_swapchain.images[frame_number % m_frames_in_flight],
                            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);
    m_swapchain.image_layout = vk::ImageLayout::ePresentSrcKHR;
}
} // namespace ving
