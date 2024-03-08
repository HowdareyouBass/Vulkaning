#include <iostream>

#include <SDL3/SDL.h>
#include <imgui.h>

#include "ving_camera.hpp"
#include "ving_core.hpp"
#include "ving_gi_renderer.hpp"
#include "ving_imgui_renderer.hpp"
#include "ving_path_tracing_renderer.hpp"
#include "ving_profilers.hpp"
#include "ving_render_frames.hpp"
#include "ving_simple_cube_renderer.hpp"
#include "ving_skybox_renderer.hpp"
#include "ving_slime_renderer.hpp"
#include "ving_vulkan_raytracer.hpp"
#include "ving_water_renderer.hpp"

const Uint8 *keys;

void run_application()
{
    if (SDL_Init(SDL_InitFlags::SDL_INIT_VIDEO) < 0)
        throw std::runtime_error(std::format("Couldn't initialize SDL: {}", SDL_GetError()));

    SDL_Window *window = SDL_CreateWindow("No title in dwm :(", 1000, 1000, SDL_WINDOW_VULKAN);
    // SDL_Window *window = SDL_CreateWindow("No title in dwm :(", 1280, 720, SDL_WINDOW_VULKAN);
    if (!window)
        throw std::runtime_error(std::format("Failed to create SDL window: {}", SDL_GetError()));

    ving::Core core{window};

    ving::Scene scene;
    scene.skybox_cubemap = ving::utils::load_cube_map("assets/textures/skies.ktx", core);
    scene.skybox_sampler = core.create_sampler(scene.skybox_cubemap.mip_levels());

    ving::Profiler profiler;

    ving::RenderFrames frames{core};
    // ving::SlimeRenderer slime_renderer{core, frames.draw_image_view()};
    // ving::SimpleCubeRenderer cube_renderer{core};
    ving::SkyboxRenderer skybox_renderer{core, scene};
    // ving::WaterRenderer water_renderer{core, scene};
    // ving::PathTracingRenderer path_tracing_renderer{core, scene};
    ving::GiRenderer gi_renderer{core};
    // ving::VulkanRaytracer vulkan_raytracer{core, frames};

    ving::ImGuiRenderer imgui_renderer{core, window};
    ving::PerspectiveCamera camera{static_cast<float>(core.get_window_extent().width) /
                                       static_cast<float>(core.get_window_extent().height),
                                   100.0f, 0.1f, glm::radians(60.0f)};

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
        // Because if you have too much fps mouse moves very little every frame
        // And SDL doesn't have precision to calculate that
        // FIXME: Fix it later

        float mouse_x = 0, mouse_y = 0;
        Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

        float halfwidth = static_cast<float>(core.get_window_extent().width) / 2.0f;
        float halfheight = static_cast<float>(core.get_window_extent().height) / 2.0f;
        if ((mouse_buttons & SDL_BUTTON_RMASK) != 0)
        {
            float mouse_x_relative_to_center = mouse_x - halfwidth;
            float mouse_y_relative_to_center = mouse_y - halfheight;

            // camera_rotate_dir.y += (mouse_x_relative_to_center > 0) - (mouse_x_relative_to_center < 0);
            // camera_rotate_dir.x += (mouse_y_relative_to_center > 0) - (mouse_y_relative_to_center < 0);
            camera_rotate_dir.y += mouse_x_relative_to_center;
            camera_rotate_dir.x += mouse_y_relative_to_center;

            SDL_WarpMouseInWindow(window, halfwidth, halfheight);
            SDL_HideCursor();
        }
        else
        {
            SDL_ShowCursor();
        }

        ving::RenderFrames::FrameInfo frame = frames.begin_frame(profiler);
        {
            ving::Task profile_camera_update{profiler, "Camera Update"};

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
            profile_camera_update.stop();

            ving::Task profile_recording{profiler, "Recording"};
            // cube_renderer.render(frame, camera);
            // imgui_renderer.render(frame, []() {});
            skybox_renderer.render(frame, camera, scene);
            // water_renderer.render(frame, camera, scene);
            // path_tracing_renderer.render(frame, camera, scene);
            gi_renderer.render(frame, camera, scene);
            // vulkan_raytracer.render(frame, camera);
            imgui_renderer.render(frame, profiler, {scene.get_imgui(), /* path_tracing_renderer.get_imgui() */});
            profile_recording.stop();
        }
        frames.end_frame(profiler);
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
