#pragma once

#include "ving_base_renderer.hpp"
#include "ving_core.hpp"
#include "ving_render_frames.hpp"

namespace ving
{
class ImGuiRenderer : public BaseRenderer
{
  public:
    ImGuiRenderer(const Core &core, SDL_Window *window);
    ~ImGuiRenderer();

    ImGuiRenderer(const ImGuiRenderer &) = delete;
    ImGuiRenderer &operator=(const ImGuiRenderer &) = delete;
    ImGuiRenderer(ImGuiRenderer &&) = delete;
    ImGuiRenderer &operator=(ImGuiRenderer &&) = delete;

    void render(const RenderFrames::FrameInfo &frame);

  private:
    vk::UniqueDescriptorPool m_pool;
};
} // namespace ving
