#include "ving_application.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>

#include "physics/ving_ray.hpp"
#include "ving_aabb_renderer.hpp"
#include "ving_gi_renderer.hpp"
#include "ving_gizmo_renderer.hpp"
#include "ving_imgui_renderer.hpp"
#include "ving_profilers.hpp"
#include "ving_skybox_renderer.hpp"

namespace ving
{
void Application::run()
{
    if (SDL_Init(SDL_InitFlags::SDL_INIT_VIDEO) < 0)
        throw std::runtime_error(std::format("Couldn't initialize SDL: {}", SDL_GetError()));

    SDL_Window *window = SDL_CreateWindow("No title in dwm :(", 1080, 1080, SDL_WINDOW_VULKAN);
    // SDL_Window *window = SDL_CreateWindow("No title in dwm :(", 1280, 720, SDL_WINDOW_VULKAN);
    if (!window)
        throw std::runtime_error(std::format("Failed to create SDL window: {}", SDL_GetError()));

    ving::Core core{window};

    ving::Scene scene;
    scene.skybox_cubemap = ving::utils::load_cube_map("assets/textures/skies.ktx", core);
    scene.skybox_sampler = core.create_sampler(scene.skybox_cubemap.mip_levels());

    std::unordered_map<uint32_t, ving::Mesh> meshes;

    meshes[0] = ving::Mesh::flat_plane(core, 100, 100, 0.05f, {});
    meshes[1] = ving::Mesh::load_from_file(core, "assets/models/smooth_vase.obj");
    meshes[2] = ving::Mesh::load_from_file(core, "assets/models/cube.obj");

    scene.objects.push_back(ving::SceneObject{meshes[1], {}});
    scene.objects.push_back(ving::SceneObject{meshes[0], {}});

    ving::Profiler profiler;

    ving::RenderFrames frames{core};
    ving::SkyboxRenderer skybox_renderer{core, scene};
    ving::GiRenderer gi_renderer{core};
    ving::GizmoRenderer gizmo_renderer{core};
    ving::AABBRenderer aabb_renderer{core};

    ving::ImGuiRenderer imgui_renderer{core, window};

    ving::PerspectiveCamera camera_1{static_cast<float>(core.get_window_extent().width) /
                                         static_cast<float>(core.get_window_extent().height),
                                     1000.0f, 0.001f, glm::radians(fov)};

    bool running = true;
    SDL_Event event;

    keys = SDL_GetKeyboardState(NULL);

    auto moving_scene_objects_imgui = [&scene]() {
        for (size_t i = 0; i < scene.objects.size(); ++i)
        {
            ImGui::Text("Obj #%zu", i);
            ImGui::DragFloat3(std::format("Position ##obj{}: ", i).data(),
                              reinterpret_cast<float *>(&scene.objects[i].transform.translation), 0.01f);
            // ImGui::Text("X:%f %f\n Y:%f %f\n Z:%f %f", scene.aabbs[i].min_x, scene.aabbs[i].max_x,
            // scene.aabbs[i].min_y,
            //             scene.aabbs[i].max_y, scene.aabbs[i].min_z, scene.aabbs[i].max_z);
        }
    };

    bool render_aabbs = false;
    auto render_aabbs_imgui = [&render_aabbs]() { ImGui::Checkbox("Render AABBs", &render_aabbs); };

    auto show_mouse_pos = [&core]() {
        float mouse_x = 0, mouse_y = 0;
        Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

        float halfwidth = static_cast<float>(core.get_window_extent().width) / 2.0f;
        float halfheight = static_cast<float>(core.get_window_extent().height) / 2.0f;

        float mouse_x_relative_to_center_remapped = (mouse_x - halfwidth) / halfwidth;
        float mouse_y_relative_to_center_remapped = (mouse_y - halfheight) / halfheight;
        mouse_y_relative_to_center_remapped *= -1;

        ImGui::Text("%f, %f", mouse_x_relative_to_center_remapped, mouse_y_relative_to_center_remapped);
    };

    // TODO:
    size_t current_object_count = scene.objects.size();
    auto remove_hits_imgiu = [current_object_count, &scene]() {
        if (ImGui::Button("Remove hits"))
        {
            // scene.objects.(scene.objects.begin(), scene.objects.begin() + current_object_count - 1);
        }
    };

    // NOTE: Idk this is kind of magic number need to investigate but for now I'll do the gizmos
    float aspect = glm::tan(glm::radians(fov));

    // NOTE: STUPID
    constexpr bool test_rays = false;
    if constexpr (test_rays)
    {
        constexpr int num_steps = 50;
        constexpr float scarcity = 4.0f;

        for (int i = 0; i < num_steps; ++i)
        {
            for (int j = 0; j < num_steps; ++j)
            {
                scene.objects.push_back(
                    ving::SceneObject{meshes[2],
                                      {{},
                                       glm::vec3{0.01f},
                                       (camera_1.position + aspect * camera_1.forward() +
                                        camera_1.right() * scarcity * (-1.0f + i * 2.0f / num_steps) +
                                        camera_1.up() * scarcity * (-1.0f + j * 2.0f / num_steps))}});
            }
        }
    }
    uint32_t hit_id = 0;
    // aabb_generator.generate(frames, scene);
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

        // Camera Movement controls
        if (keys[SDL_SCANCODE_W])
            camera_direction.z += 1;
        if (keys[SDL_SCANCODE_S])
            camera_direction.z += -1;
        if (keys[SDL_SCANCODE_D])
            camera_direction.x += 1;
        if (keys[SDL_SCANCODE_A])
            camera_direction.x += -1;
        if (keys[SDL_SCANCODE_SPACE])
            camera_direction.y += 1;
        if (keys[SDL_SCANCODE_C])
            camera_direction.y += -1;

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

            camera_rotate_dir.y += mouse_x_relative_to_center;
            camera_rotate_dir.x += mouse_y_relative_to_center;

            SDL_WarpMouseInWindow(window, halfwidth, halfheight);
            SDL_HideCursor();
        }
        else
        {
            SDL_ShowCursor();
        }

        // Object choosing
        if ((mouse_buttons & SDL_BUTTON_LMASK) != 0)
        {
            float mouse_x_relative_to_center_remapped = (mouse_x - halfwidth) / halfwidth;
            float mouse_y_relative_to_center_remapped = (mouse_y - halfheight) / halfheight;
            mouse_y_relative_to_center_remapped *= -1;

            auto hit = ving::raycast_scene(camera_1.position,
                                           glm::normalize(aspect * camera_1.forward() +
                                                          mouse_x_relative_to_center_remapped * camera_1.right() +
                                                          mouse_y_relative_to_center_remapped * camera_1.up()),
                                           scene);
            if (hit.first)
            {
                hit_id = hit.second.object_id;
                if constexpr (test_rays)
                {
                    scene.objects.push_back(
                        ving::SceneObject{meshes[2], ving::Transform{{}, glm::vec3{0.01f}, hit.second.position}});
                }
            }
        }

        ving::PerspectiveCamera &camera = camera_1;

        ving::RenderFrames::FrameInfo frame = frames.begin_frame(profiler);
        {
            camera.position += camera.right() * camera_direction.x * frame.delta_time * camera.move_speed;
            // NOTE: Camera Position is in vulkan space
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

            ving::Task profile_recording{profiler, "Recording"};
            skybox_renderer.render(frame, camera, scene);
            gi_renderer.render(frame, camera, scene);
            gizmo_renderer.render(frame, camera, scene.objects[0]);
            if (render_aabbs)
                aabb_renderer.render_all(frame, camera, scene);
            else
                aabb_renderer.render_single(frame, camera, scene, hit_id);

            imgui_renderer.render(frame, profiler,
                                  {
                                      render_aabbs_imgui,
                                      show_mouse_pos,
                                      scene.get_imgui(),
                                      gi_renderer.get_imgui(),
                                      moving_scene_objects_imgui,
                                  });

            profile_recording.stop();
        }
        frames.end_frame(profiler);
    }

    core.wait_idle();
}
} // namespace ving
