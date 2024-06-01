#include "ving_render_frames.hpp"
#include "ving_profilers.hpp"

namespace ving
{
RenderFrames::RenderFrames(const Core &core) : r_core{core}, m_present_queue{core, frames_in_flight}
{
    m_graphics_queue = core.get_graphics_queue();

    m_draw_image =
        core.create_image2d(vk::Extent3D{core.get_window_extent(), 1}, vk::Format::eR16G16B16A16Sfloat,
                            vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst |
                                vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment,
                            vk::ImageLayout::eUndefined);

    for (auto &&frame : m_frames)
    {
        frame.commands = std::move(core.allocate_command_buffers(1)[0]);

        frame.render_fence = core.create_fence(true);
        frame.image_acquired_semaphore = core.create_semaphore();
        frame.render_finished_semaphore = core.create_semaphore();
    }

    m_immediate_submit_fence = core.create_fence(true);

    auto allocated_command_buffers = core.allocate_command_buffers(1);
    m_immediate_submit_commands = std::move(allocated_command_buffers.front());
}

RenderFrames::FrameInfo RenderFrames::begin_frame(Profiler &profiler)
{
    m_start_time = std::chrono::high_resolution_clock::now();
    FrameResources &cur_frame = m_frames[m_frame_number % frames_in_flight];

    {
        auto task = profiler.start_scoped_task("Wait For Fence");
        r_core.wait_for_fence(cur_frame.render_fence.get());
    }
    r_core.reset_fence(cur_frame.render_fence.get());

    {
        auto task = profiler.start_scoped_task("Acquire Image");
        m_present_queue.acquire_image(cur_frame.image_acquired_semaphore.get());
    }

    vk::CommandBuffer &cmd = cur_frame.commands.get();

    cmd.reset();

    auto begin_info = vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    vk::detail::resultCheck(cmd.begin(&begin_info), "Command buffer begin failed");

    return FrameInfo{cmd, m_draw_image, m_frame_number, m_delta_time, m_time};
}
void RenderFrames::end_frame(Profiler &profiler)
{
    auto task = profiler.start_scoped_task("End Frame");

    FrameResources &cur_frame = m_frames[m_frame_number % frames_in_flight];
    vk::CommandBuffer &cmd = cur_frame.commands.get();

    m_draw_image.transition_layout(cmd, vk::ImageLayout::eTransferSrcOptimal);

    m_present_queue.copy_image_to_swapchain(cmd, m_draw_image.image(), m_draw_image.extent(), m_frame_number);
    m_present_queue.transition_swapchain_image_to_present(cmd, m_frame_number);

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

    {
        auto task = profiler.start_scoped_task("Submit");
        m_graphics_queue.submit2(submit, cur_frame.render_fence.get());
    }
    {
        auto task = profiler.start_scoped_task("Present");
        m_present_queue.present_image(cur_frame.render_finished_semaphore.get(), m_frame_number);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta = end_time - m_start_time;
    m_delta_time = delta.count() * 1000.0f;
    m_time += m_delta_time;
    m_start_time = end_time;

    ++m_frame_number;
}
void RenderFrames::immediate_submit(std::function<void(vk::CommandBuffer)> &&function)
{
    r_core.reset_fence(m_immediate_submit_fence.get());
    m_immediate_submit_commands->reset();

    vk::CommandBuffer cmd = m_immediate_submit_commands.get();

    auto cmd_begin_info = vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd.begin(cmd_begin_info);
    function(cmd);
    cmd.end();

    auto cmd_info = vk::CommandBufferSubmitInfo{}.setCommandBuffer(cmd);
    auto submit = vk::SubmitInfo2{}.setCommandBufferInfos(cmd_info);

    m_graphics_queue.submit2(submit, m_immediate_submit_fence.get());

    r_core.wait_for_fence(m_immediate_submit_fence.get());
}
} // namespace ving
