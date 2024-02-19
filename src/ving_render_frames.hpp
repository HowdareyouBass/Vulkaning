#pragma once

#include <chrono>

#include "ving_core.hpp"

#include "ving_present_queue.hpp"

namespace ving
{
class Profiler;

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
        Image2D &draw_image;
        uint64_t frame_number;
        float delta_time;
        float time;
    };

    // HARD: Let the user decide
    static constexpr int frames_in_flight = 2;

    RenderFrames(const Core &core);
    ~RenderFrames() = default;

    RenderFrames(const RenderFrames &) = delete;
    RenderFrames &operator=(const RenderFrames &) = delete;
    RenderFrames(RenderFrames &&) = delete;
    RenderFrames &operator=(RenderFrames &&) = delete;

    FrameInfo begin_frame(Profiler &profiler);
    void end_frame(Profiler &profiler);

    vk::ImageView draw_image_view() { return m_draw_image.view(); }

  private:
    // HACK: I don't need this to be copyable or movable for now so i will stick to reference
    // Maybe later use shared or weak ptr???
    const Core &r_core;

    Image2D m_draw_image;
    PresentQueue m_present_queue;

    std::array<FrameResources, frames_in_flight> m_frames;
    uint64_t m_frame_number{0};

    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
    float m_delta_time{};
    float m_time{};
};
} // namespace ving
