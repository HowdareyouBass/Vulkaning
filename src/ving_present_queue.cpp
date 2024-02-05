#include "ving_present_queue.hpp"
#include "ving_utils.hpp"

namespace ving
{
PresentQueue::PresentQueue(const Core &core, uint32_t frames_in_flight_count) : r_core{core}
{
    m_swapchain = core.create_swapchain(frames_in_flight_count);
    m_present = core.get_present_queue();
}

uint32_t PresentQueue::acquire_image(vk::Semaphore image_acquire_semaphore)
{
    return r_core.acquire_swapchain_image(*m_swapchain.swapchain, image_acquire_semaphore);
}

void PresentQueue::present_image(vk::Semaphore wait_semaphore, uint32_t image_index)
{
    auto present_info = vk::PresentInfoKHR{}
                            .setSwapchains(m_swapchain.swapchain.get())
                            .setWaitSemaphores(wait_semaphore)
                            .setImageIndices(image_index);

    vk::resultCheck(m_present.presentKHR(&present_info), "Present image failed");
}
void PresentQueue::transition_swapchain_to_present(vk::CommandBuffer cmd, uint32_t index)
{
    utils::transition_image(cmd, m_swapchain.images[index], vk::ImageLayout::eUndefined,
                            vk::ImageLayout::ePresentSrcKHR);
    m_swapchain.image_layout = vk::ImageLayout::ePresentSrcKHR;
}
} // namespace ving
