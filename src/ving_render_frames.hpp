#pragma once

#include "ving_core.hpp"

#include "ving_present_queue.hpp"

namespace ving
{
class RenderFrames
{
  private:
    struct FrameResources
    {
        vk::UniqueCommandBuffer commands;

        vk::UniqueSemaphore image_acquired_semaphore, render_finished_semaphore;
        vk::UniqueFence render_fence;
    };

  public:
    struct FrameInfo
    {
        vk::CommandBuffer cmd;
    };

    // HARD: Let the user decide
    static constexpr int frames_in_flight = 2;

    RenderFrames(const Core &core);
    ~RenderFrames() = default;

    RenderFrames(const RenderFrames &) = delete;
    RenderFrames &operator=(const RenderFrames &) = delete;
    RenderFrames(RenderFrames &&) = delete;
    RenderFrames &operator=(RenderFrames &&) = delete;

    FrameInfo begin_frame();
    void end_frame();

  private:
    // HACK: I don't need this to be copyable or movable for now so i will stick to reference
    // Maybe later use shared or weak ptr???
    const Core &r_core;

    PresentQueue m_present_queue;

    std::array<FrameResources, frames_in_flight> m_frames;
    uint64_t m_frame_number{0};
};
} // namespace ving
