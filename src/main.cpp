#include <iostream>

#include <SDL3/SDL.h>

#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_imgui_renderer.hpp"
#include "ving_render_frames.hpp"
#include "ving_simple_cube_renderer.hpp"
#include "ving_skybox_renderer.hpp"
#include "ving_slime_renderer.hpp"
#include "ving_water_renderer.hpp"

const Uint8 *keys;

void run_application()
{
    if (SDL_Init(SDL_InitFlags::SDL_INIT_VIDEO) < 0)
        throw std::runtime_error(std::format("Couldn't initialize SDL: {}", SDL_GetError()));

    SDL_Window *window = SDL_CreateWindow("No title in dwm :(", 1000, 1000, SDL_WINDOW_VULKAN);
    if (!window)
        throw std::runtime_error(std::format("Failed to create SDL window: {}", SDL_GetError()));

    ving::Core core{window};
    ving::RenderFrames frames{core};
    // ving::SlimeRenderer slime_renderer{core, frames.draw_image_view()};
    ving::SimpleCubeRenderer cube_renderer{core};
    ving::SkyboxRenderer skybox_renderer{core};
    ving::WaterRenderer water_renderer{core};
    ving::ImGuiRenderer imgui_renderer{core, window};
    ving::PerspectiveCamera camera{static_cast<float>(core.get_window_extent().width) /
                                       static_cast<float>(core.get_window_extent().height),
                                   100.0f, 0.1f, glm::radians(60.0f)};
    camera.position = {0.0f, 1.5f, -5.0f};

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
            imgui_renderer.process_sdl_event(event);
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

        // Camera rotation controls
        if (keys[SDL_SCANCODE_UP])
            camera_rotate_dir.x -= 1.0f;
        if (keys[SDL_SCANCODE_DOWN])
            camera_rotate_dir.x += 1.0f;
        if (keys[SDL_SCANCODE_RIGHT])
            camera_rotate_dir.y += 1.0f;
        if (keys[SDL_SCANCODE_LEFT])
            camera_rotate_dir.y -= 1.0f;

        // NOTE: This method freaks out if you have too much fps
        float mouse_x = 0, mouse_y = 0;
        Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
        float halfwidth = static_cast<float>(core.get_window_extent().width) / 2.0f;
        float halfheight = static_cast<float>(core.get_window_extent().height) / 2.0f;
        if ((mouse_buttons & SDL_BUTTON_RMASK) != 0)
        {
            float mouse_x_relative_to_center = mouse_x - halfwidth;
            float mouse_y_relative_to_center = mouse_y - halfheight;
            // if (std::abs(mouse_x_relative_to_center) > std::numeric_limits<float>::epsilon() &&
            //     std::abs(mouse_y_relative_to_center) > std::numeric_limits<float>::epsilon())
            // {
            //     SDL_Log("%f %f\n", mouse_x, mouse_y);
            //     SDL_Log("%f %f\n", mouse_x_relative_to_center, mouse_y_relative_to_center);
            // }

            camera_rotate_dir.y += mouse_x_relative_to_center;
            camera_rotate_dir.x += mouse_y_relative_to_center;

            SDL_WarpMouseInWindow(window, halfwidth, halfheight);
            SDL_HideCursor();
        }
        else
        {
            SDL_ShowCursor();
        }

        ving::RenderFrames::FrameInfo frame = frames.begin_frame();
        {
            camera.position += camera.right() * camera_direction.x * frame.delta_time * camera.move_speed;
            camera.position += camera.up() * camera_direction.y * frame.delta_time * camera.move_speed;
            camera.position += camera.forward() * camera_direction.z * frame.delta_time * camera.move_speed;
            if (glm::dot(camera_rotate_dir, camera_rotate_dir) > std::numeric_limits<float>::epsilon())
            {
                camera.rotation +=
                    camera_rotate_dir * frame.delta_time *
                    (((mouse_buttons & SDL_BUTTON_RMASK) != 0) ? camera.mouse_look_speed : camera.arrows_look_speed);
                camera.rotation.x = glm::clamp(camera.rotation.x, -1.5f, 1.5f);
                camera.rotation.y = glm::mod(camera.rotation.y, glm::two_pi<float>());
            }
            camera.update();

            // cube_renderer.render(frame, camera);
            // imgui_renderer.render(frame, []() {});
            skybox_renderer.render(frame, camera);
            auto imgui_frame = water_renderer.render(frame, camera);
            imgui_renderer.render(frame, std::move(imgui_frame));
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
