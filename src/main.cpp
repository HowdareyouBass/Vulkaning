#include <iostream>

#include <SDL3/SDL.h>

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include <imgui.h>

#include "ving_camera.hpp"
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

constexpr float camera_speed = 0.01f;
constexpr float camera_look_speed = 0.002f;

const Uint8 *keys;

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
    ving::Camera camera;
    camera.aspect =
        static_cast<float>(core.get_window_extent().width) / static_cast<float>(core.get_window_extent().height);
    camera.far = 0.1f;
    camera.near = 100.0f;
    camera.fov = glm::radians(60.0f);
    camera.set_perspective_projection();
    camera.set_view_direction(glm::vec3{0.0f, 0.0f, 1.0f});

    bool running = true;
    SDL_Event event;

    keys = SDL_GetKeyboardState(NULL);

    while (running)
    {
        glm::vec3 camera_direction{0.0f, 0.0f, 0.0f};
        glm::vec3 camera_rotate_dir{0.0f, 0.0f, 0.0f};
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT: {
                running = false;
                break;
            }
            default:
                break;
            }
            ImGui_ImplSDL3_ProcessEvent(&event);
        }

        if (keys[SDL_SCANCODE_W])
            camera_direction.z = 1;
        if (keys[SDL_SCANCODE_S])
            camera_direction.z = -1;
        if (keys[SDL_SCANCODE_D])
            camera_direction.x = 1;
        if (keys[SDL_SCANCODE_A])
            camera_direction.x = -1;
        if (keys[SDL_SCANCODE_SPACE])
            camera_direction.y = 1;
        if (keys[SDL_SCANCODE_C])
            camera_direction.y = -1;

        // HACK: Had to reverse the up and down keys idk why
        // I really need deeper understaning of this
        if (keys[SDL_SCANCODE_UP])
            camera_rotate_dir.x -= 1.0f;
        if (keys[SDL_SCANCODE_DOWN])
            camera_rotate_dir.x += 1.0f;
        if (keys[SDL_SCANCODE_RIGHT])
            camera_rotate_dir.y += 1.0f;
        if (keys[SDL_SCANCODE_LEFT])
            camera_rotate_dir.y -= 1.0f;

        ving::RenderFrames::FrameInfo frame = frames.begin_frame();
        {
            camera.position += camera.right() * camera_direction.x * frame.delta_time * camera_speed;
            camera.position += camera.up() * camera_direction.y * frame.delta_time * camera_speed;
            if (glm::dot(camera_rotate_dir, camera_rotate_dir) > std::numeric_limits<float>::epsilon())
            {
                camera.rotation += camera_rotate_dir * frame.delta_time * camera_look_speed;
            }
            // camera.position += glm::vec3{0.0f, -1.0f, 0.0f} * camera_direction.y * frame.delta_time * camera_speed;
            camera.position += camera.forward() * camera_direction.z * frame.delta_time * camera_speed;
            // camera.set_view_direction(glm::vec3{0.0f, 0.0f, 1.0f});
            camera.set_view_YXZ();
            // slime_renderer.render(frame);
            cube_renderer.render(frame, camera);
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
