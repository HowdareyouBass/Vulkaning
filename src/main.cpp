#include <iostream>

#include <SDL3/SDL.h>

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include <imgui.h>

#include "ving_core.hpp"
#include "ving_engine.hpp"
#include "ving_imgui_renderer.hpp"
#include "ving_render_frames.hpp"
#include "ving_simple_cube_renderer.hpp"
#include "ving_slime_renderer.hpp"

namespace ving
{
struct ImguiScopedFrame
{
    ImguiScopedFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }
    ~ImguiScopedFrame() { ImGui::EndFrame(); }
};
} // namespace ving

void run_application()
{
    if (SDL_Init(SDL_InitFlags::SDL_INIT_VIDEO) < 0)
        throw std::runtime_error(std::format("Couldn't initialize SDL: {}", SDL_GetError()));

    SDL_Window *window = SDL_CreateWindow("No title in dwm :(", 800, 800, SDL_WINDOW_VULKAN);
    if (!window)
        throw std::runtime_error(std::format("Failed to create SDL window: {}", SDL_GetError()));

    ving::Core core{window};
    ving::RenderFrames frames{core};
    ving::SlimeRenderer slime_renderer{core, frames.draw_image_view()};
    ving::SimpleCubeRenderer cube_renderer{core};
    ving::ImGuiRenderer imgui_renderer{core, window};

    bool running = true;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT: {
                running = false;
                break;
            }
            }
            ImGui_ImplSDL3_ProcessEvent(&event);
        }

        ving::RenderFrames::FrameInfo frame = frames.begin_frame();
        {
            // slime_renderer.render(frame);
            cube_renderer.render(frame);
            imgui_renderer.render(frame);
            // ving::ImguiScopedFrame frame{};
        }
        frames.end_frame();
    }

    core.wait_idle();
}

int main()
{
    try
    {
        run_application();
        // ving::Engine engine{};
        // engine.run();
    }
    catch (vk::SystemError &e)
    {
        std::cerr << "Vulkan Exception thrown: " << e.what() << "\n";
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception thrown: " << e.what() << "\n";
    }
    catch (...)
    {
        std::cerr << "Unkown error!\n";
    }
}
