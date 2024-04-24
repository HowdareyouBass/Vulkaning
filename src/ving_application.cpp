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
// Such a mess :(
Application::Application(SDL_Window *window)
    : m_window{window}, m_core{m_window}, m_scene{m_core, "assets/textures/skies.ktx"},
      m_camera{m_core.get_aspect(), 1000.0f, 0.001f, glm::radians(fov)}, m_frames{m_core},
      m_imgui_renderer{m_core, m_window}, m_skybox_renderer{m_core, m_scene}, m_gi_renderer{m_core},
      m_gizmo_renderer{m_core}, m_aabb_renderer{m_core}
{
    m_render_aabbs_checkbox_imgui = [this]() { ImGui::Checkbox("Render AABBs", &m_render_aabbs); };
    m_moving_scene_objects_imgui = [this]() {
        for (size_t i = 0; i < m_scene.objects.size(); ++i)
        {
            ImGui::Text("Obj #%zu", i);
            ImGui::DragFloat3(std::format("Position ##obj{}: ", i).data(),
                              reinterpret_cast<float *>(&m_scene.objects[i].transform.translation), 0.01f);
            // ImGui::Text("X:%f %f\n Y:%f %f\n Z:%f %f", m_scene.aabbs[i].min_x, scene.aabbs[i].max_x,
            // m_scene.aabbs[i].min_y,
            //             m_scene.aabbs[i].max_y, scene.aabbs[i].min_z, scene.aabbs[i].max_z);
        }
    };
    m_show_mouse_pos = [this]() {
        float mouse_x = 0, mouse_y = 0;
        Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

        float halfwidth = static_cast<float>(m_core.get_window_extent().width) / 2.0f;
        float halfheight = static_cast<float>(m_core.get_window_extent().height) / 2.0f;

        float mouse_x_relative_to_center_remapped = (mouse_x - halfwidth) / halfwidth;
        float mouse_y_relative_to_center_remapped = (mouse_y - halfheight) / halfheight;
        mouse_y_relative_to_center_remapped *= -1;

        ImGui::Text("%f, %f", mouse_x_relative_to_center_remapped, mouse_y_relative_to_center_remapped);
    };

    m_show_debug_bool = [this]() { ImGui::Text("%b", m_debug_bool); };
}
void Application::run()
{
    m_meshes[0] = ving::Mesh::flat_plane(m_core, 100, 100, 0.05f, {});
    m_meshes[1] = ving::Mesh::load_from_file(m_core, "assets/models/smooth_vase.obj");
    m_meshes[2] = ving::Mesh::load_from_file(m_core, "assets/models/cube.obj");

    m_scene.objects.push_back(ving::SceneObject{m_meshes[1], {}});
    m_scene.objects.push_back(ving::SceneObject{m_meshes[0], {}});

    keys = SDL_GetKeyboardState(NULL);

    // NOTE: STUPID
    constexpr bool test_rays = false;
    constexpr int num_steps = 50;
    constexpr float scarcity = 4.0f;

    if constexpr (test_rays)
    {

        for (int i = 0; i < num_steps; ++i)
        {
            for (int j = 0; j < num_steps; ++j)
            {
                m_scene.objects.push_back(ving::SceneObject{
                    m_meshes[2],
                    {{},
                     glm::vec3{0.01f},
                     glm::normalize(m_camera.position + glm::tan(glm::radians(fov)) * m_camera.forward() +
                                    m_camera.right() * scarcity * (-1.0f + i * 2.0f / num_steps) +
                                    m_camera.up() * scarcity * (-1.0f + j * 2.0f / num_steps))}});
            }
        }
    }
    m_running = true;
}

void Application::update()
{
    SDL_Event event;

    glm::vec3 camera_direction{0.0f, 0.0f, 0.0f};
    glm::vec3 camera_rotate_dir{0.0f, 0.0f, 0.0f};

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT: {
            m_running = false;
            break;
        }
        default:
            break;
        }
        m_imgui_renderer.process_sdl_event(event);
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

    // Camera rotation
    // NOTE: This method freaks out if you have too much fps
    // Because if you have too much fps mouse moves very little every frame
    // And SDL doesn't have precision to calculate that
    // FIXME: Fix it later

    float mouse_x = 0, mouse_y = 0;
    Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

    float halfwidth = static_cast<float>(m_core.get_window_extent().width) / 2.0f;
    float halfheight = static_cast<float>(m_core.get_window_extent().height) / 2.0f;

    if ((mouse_buttons & SDL_BUTTON_RMASK) != 0)
    {
        float mouse_x_relative_to_center = mouse_x - halfwidth;
        float mouse_y_relative_to_center = mouse_y - halfheight;

        camera_rotate_dir.y += mouse_x_relative_to_center;
        camera_rotate_dir.x += mouse_y_relative_to_center;

        SDL_WarpMouseInWindow(m_window, halfwidth, halfheight);
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
        float mouse_y_relative_to_center_remapped = -(mouse_y - halfheight) / halfheight;

        auto gizmo_hit =
            ving::raycast_gizmos({mouse_x_relative_to_center_remapped, mouse_y_relative_to_center_remapped}, m_camera,
                                 m_scene.objects[m_hit_id]);

        if (gizmo_hit.first)
        {
            m_debug_bool = true;
        }
        else
        {
            m_debug_bool = false;
            auto hit = ving::raycast_scene(m_camera.position,
                                           glm::normalize(glm::tan(glm::radians(fov)) * m_camera.forward() +
                                                          mouse_x_relative_to_center_remapped * m_camera.right() +
                                                          mouse_y_relative_to_center_remapped * m_camera.up()),
                                           m_scene);

            if (hit.first)
            {
                m_hit_id = hit.second.object_id;
                if constexpr (test_rays)
                {
                    m_scene.objects.push_back(
                        ving::SceneObject{m_meshes[2], ving::Transform{{}, glm::vec3{0.01f}, hit.second.position}});
                }
            }
        }
    }

    ving::RenderFrames::FrameInfo frame = m_frames.begin_frame(m_profiler);
    {
        // Moving the camera
        m_camera.position += m_camera.right() * camera_direction.x * frame.delta_time * m_camera.move_speed;
        m_camera.position += m_camera.up() * camera_direction.y * frame.delta_time * m_camera.move_speed;
        m_camera.position += m_camera.forward() * camera_direction.z * frame.delta_time * m_camera.move_speed;

        // Rotating the camera
        if (glm::dot(camera_rotate_dir, camera_rotate_dir) > std::numeric_limits<float>::epsilon())
        {
            m_camera.rotation +=
                camera_rotate_dir * frame.delta_time *
                (((mouse_buttons & SDL_BUTTON_RMASK) != 0) ? m_camera.mouse_look_speed : m_camera.arrows_look_speed);
            m_camera.rotation.x = glm::clamp(m_camera.rotation.x, -1.5f, 1.5f);
            m_camera.rotation.y = glm::mod(m_camera.rotation.y, glm::two_pi<float>());
        }
        m_camera.update();

        ving::Task profile_recording{m_profiler, "Recording"};
        m_skybox_renderer.render(frame, m_camera, m_scene);
        m_gi_renderer.render(frame, m_camera, m_scene);

        if (m_render_aabbs)
            m_aabb_renderer.render_scene(frame, m_camera, m_scene);
        else
            m_aabb_renderer.render_object_aabb(frame, m_camera, m_scene.objects[m_hit_id]);

        m_aabb_renderer.render_gizmo_aabb(frame, m_camera, m_scene.objects[m_hit_id]);

        m_gizmo_renderer.render(frame, m_camera, m_scene.objects[m_hit_id]);

        m_imgui_renderer.render(frame, m_profiler,
                                {
                                    m_render_aabbs_checkbox_imgui,
                                    m_show_debug_bool,
                                    m_show_mouse_pos,
                                    m_scene.get_imgui(),
                                    m_gi_renderer.get_imgui(),
                                    m_moving_scene_objects_imgui,
                                });

        profile_recording.stop();
    }
    m_frames.end_frame(m_profiler);
}
void Application::stop()
{
    m_core.wait_idle();
}
} // namespace ving
