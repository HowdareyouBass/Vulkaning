#pragma once

#include "ving_base_renderer.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"

union SDL_Event;

namespace ving
{
class Profiler;

struct ImGuiFrame
{
    std::vector<std::function<void()>> functions;
};

class ImGuiRenderer : public BaseRenderer
{
  public:
    ImGuiRenderer(const Core &core, SDL_Window *window);
    ~ImGuiRenderer();

    ImGuiRenderer(const ImGuiRenderer &) = delete;
    ImGuiRenderer &operator=(const ImGuiRenderer &) = delete;
    ImGuiRenderer(ImGuiRenderer &&) = delete;
    ImGuiRenderer &operator=(ImGuiRenderer &&) = delete;

    void render(const RenderFrames::FrameInfo &frame, Profiler &profiler,
                const std::vector<std::function<void()>> &imgui_frames);
    void process_sdl_event(const SDL_Event &event);

  private:
    vk::UniqueDescriptorPool m_pool;
};
} // namespace ving
