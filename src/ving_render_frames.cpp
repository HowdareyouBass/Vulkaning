#include "ving_render_frames.hpp"

namespace ving
{
RenderFrames::RenderFrames(const Core &core) : r_core{core}, m_present_queue{core, frames_in_flight}
{
    for (auto &&frame : m_frames)
    {
        frame.commands = std::move(core.allocate_command_buffers(1)[0]);

        frame.render_fence = core.create_fence(true);
        frame.image_acquired_semaphore = core.create_semaphore();
        frame.render_finished_semaphore = core.create_semaphore();
    }
}

RenderFrames::FrameInfo RenderFrames::begin_frame()
{
    FrameResources &cur_frame = m_frames[m_frame_number % frames_in_flight];

    r_core.wait_for_fence(cur_frame.render_fence.get());
    r_core.reset_fence(cur_frame.render_fence.get());

    m_present_queue.acquire_image(cur_frame.image_acquired_semaphore.get());

    vk::CommandBuffer &cmd = cur_frame.commands.get();

    cmd.reset();

    auto begin_info = vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    vk::resultCheck(cmd.begin(&begin_info), "Command buffer begin failed");

    return FrameInfo{cmd};
}
void RenderFrames::end_frame()
{
    FrameResources &cur_frame = m_frames[m_frame_number % frames_in_flight];
    vk::CommandBuffer &cmd = cur_frame.commands.get();

    m_present_queue.transition_swapchain_to_present(cmd, m_frame_number % frames_in_flight);

    cmd.end();

    auto cmd_info = vk::CommandBufferSubmitInfo{}.setCommandBuffer(cmd);
    auto signal_info = vk::SemaphoreSubmitInfo{}
                           .setSemaphore(cur_frame.render_finished_semaphore.get())
                           .setStageMask(vk::PipelineStageFlagBits2::eAllCommands)
                           .setValue(1);
    auto wait_info = vk::SemaphoreSubmitInfo{}
                         .setSemaphore(cur_frame.image_acquired_semaphore.get())
                         .setStageMask(vk::PipelineStageFlagBits2::eAllCommands)
                         .setValue(1);

    auto submit = vk::SubmitInfo2{}
                      .setWaitSemaphoreInfos(wait_info)
                      .setSignalSemaphoreInfos(signal_info)
                      .setCommandBufferInfos(cmd_info);

    r_core.get_graphics_queue().submit2(submit, cur_frame.render_fence.get());

    m_present_queue.present_image(cur_frame.render_finished_semaphore.get(), m_frame_number % frames_in_flight);

    ++m_frame_number;
}
} // namespace ving
