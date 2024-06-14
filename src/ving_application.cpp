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
    m_window_info.width = m_core.get_window_extent().width;
    m_window_info.height = m_core.get_window_extent().height;

    m_render_aabbs_checkbox_imgui = [this]() { ImGui::Checkbox("Render AABBs", &m_render_aabbs); };
    m_moving_scene_objects_imgui = [this]() {
        for (size_t i = 0; i < m_scene.objects.size(); ++i)
        {
            ImGui::Text("Obj #%zu", i);
            ImGui::DragFloat3(std::format("Position ##obj{}: ", i).data(),
                              reinterpret_cast<float *>(&m_scene.objects[i].transform.translation), 0.01f);
            ImGui::DragFloat3(std::format("Rotation ##obj{}: ", i).data(),
                              reinterpret_cast<float *>(&m_scene.objects[i].transform.rotation), 0.01f);

            // NOTE: To show world space object aabb
            //
            // AABB world_space_aabbs = m_scene.objects[i].get_world_space_aabb();
            //
            // ImGui::Text("Min: %f %f %f", world_space_aabbs.min.x, world_space_aabbs.min.y, world_space_aabbs.min.z);
            // ImGui::Text("Max: %f %f %f", world_space_aabbs.max.x, world_space_aabbs.max.y, world_space_aabbs.max.z);

            // ImGui::Text("X:%f %f\n Y:%f %f\n Z:%f %f", m_scene.aabbs[i].min_x, scene.aabbs[i].max_x,
            // m_scene.aabbs[i].min_y,
            //             m_scene.aabbs[i].max_y, scene.aabbs[i].min_z, scene.aabbs[i].max_z);
        }
    };
    m_show_mouse_pos = [this]() {
        float mouse_x = 0, mouse_y = 0;
        // Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

        float halfwidth = static_cast<float>(m_core.get_window_extent().width) / 2.0f;
        float halfheight = static_cast<float>(m_core.get_window_extent().height) / 2.0f;

        float mouse_x_relative_to_center_remapped = (mouse_x - halfwidth) / halfwidth;
        float mouse_y_relative_to_center_remapped = (mouse_y - halfheight) / halfheight;
        mouse_y_relative_to_center_remapped *= -1;

        ImGui::Text("%f, %f", mouse_x_relative_to_center_remapped, mouse_y_relative_to_center_remapped);
    };

    m_show_debug_bool = [this]() { ImGui::Text("%b", m_debug_bool); };
    m_show_log_string = [this]() { ImGui::Text("%s", m_log_string.c_str()); };
}
void Application::run()
{
    m_meshes[MeshType::Plane] = ving::Mesh::flat_plane(m_core, 100, 100, 0.05f, {});
    m_meshes[MeshType::SmoothVase] = ving::Mesh::load_from_file(m_core, "assets/models/smooth_vase.obj");
    m_meshes[MeshType::Cube] = ving::Mesh::load_from_file(m_core, "assets/models/cube.obj");
    m_meshes[MeshType::Dragon] = ving::Mesh::load_from_file(m_core, "assets/models/DragonAttenuation.glb");

    m_scene.objects.push_back(ving::SceneObject{m_meshes[MeshType::SmoothVase], {}});
    m_scene.objects.push_back(
        ving::SceneObject{m_meshes[MeshType::Plane], {glm::vec3{0.0f}, glm::vec3{1.0f}, glm::vec3{0.0f, 1.0f, 0.0f}}});
    m_scene.objects.push_back(ving::SceneObject{m_meshes[MeshType::Dragon], {}});

    keys = SDL_GetKeyboardState(NULL);

    m_running = true;
}

void Application::update()
{
    SDL_Event event;

    glm::vec3 camera_direction{0.0f, 0.0f, 0.0f};
    glm::vec3 camera_rotate_dir{0.0f, 0.0f, 0.0f};

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            m_running = false;
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

    update_mouse_info();

    // Camera rotation
    // NOTE: This method freaks out if you have too much fps
    // Because if you have too much fps mouse moves very little every frame
    // And SDL doesn't have precision to calculate that
    // FIXME: Fix it later

    if ((m_mouse_info.buttons & SDL_BUTTON_RMASK) != 0)
    {
        camera_rotate_dir.y += m_mouse_info.pixel_relative_to_center_coord.x;
        camera_rotate_dir.x += m_mouse_info.pixel_relative_to_center_coord.y;

        SDL_WarpMouseInWindow(m_window, m_window_info.width / 2.0f, m_window_info.height / 2.0f);
        SDL_HideCursor();
    }
    else
    {
        SDL_ShowCursor();
    }

    // Object choosing and gizmos
    // FIXME: Refactor later

    if ((m_mouse_info.buttons & SDL_BUTTON_LMASK) != 0)
    {
        update_gizmo();
        select_scene_object();
    }
    else
    {
        m_locked = false;
    }
    // m_debug_bool = m_locked;

    ving::RenderFrames::FrameInfo frame = m_frames.begin_frame(m_profiler);
    {
        // Moving the camera
        m_camera.position += m_camera.right() * camera_direction.x * frame.delta_time * m_camera.move_speed;
        m_camera.position += m_camera.up() * camera_direction.y * frame.delta_time * m_camera.move_speed;
        m_camera.position += m_camera.forward() * camera_direction.z * frame.delta_time * m_camera.move_speed;

        // Rotating the camera
        if (glm::dot(camera_rotate_dir, camera_rotate_dir) > std::numeric_limits<float>::epsilon())
        {
            m_camera.rotation += camera_rotate_dir * frame.delta_time *
                                 (((m_mouse_info.buttons & SDL_BUTTON_RMASK) != 0) ? m_camera.mouse_look_speed
                                                                                   : m_camera.arrows_look_speed);
            m_camera.rotation.x = glm::clamp(m_camera.rotation.x, -1.5f, 1.5f);
            m_camera.rotation.y = glm::mod(m_camera.rotation.y, glm::two_pi<float>());
        }
        m_camera.update();

        ving::Task profile_recording{m_profiler, "Recording Command Buffers"};
        m_skybox_renderer.render(frame, m_camera, m_scene);
        m_gi_renderer.render(frame, m_camera, m_scene);

        // NOTE: Shows focused object aabb
        if (m_render_aabbs)
            m_aabb_renderer.render_scene(frame, m_camera, m_scene);
        // else
        //     m_aabb_renderer.render_object_aabb(frame, m_camera, m_scene.objects[m_focused_object]);

        m_gizmo_renderer.render(frame, m_camera, m_scene.objects[m_focused_object]);

        m_imgui_renderer.render(frame, m_profiler,
                                {
                                    m_render_aabbs_checkbox_imgui,
                                    m_show_debug_bool,
                                    m_show_mouse_pos,
                                    m_show_log_string,
                                    m_scene.get_imgui(),
                                    m_gi_renderer.get_imgui(),
                                    m_moving_scene_objects_imgui,
                                });

        profile_recording.stop();
    }
    m_frames.end_frame(m_profiler);

    m_log_string.clear();
}
void Application::stop()
{
    m_core.wait_idle();
}

void Application::update_mouse_info()
{
    m_mouse_info.buttons = SDL_GetMouseState(&m_mouse_info.pixel_coord.x, &m_mouse_info.pixel_coord.y);

    m_mouse_info.pixel_relative_to_center_coord.x = m_mouse_info.pixel_coord.x - m_window_info.width / 2.0f;
    m_mouse_info.pixel_relative_to_center_coord.y = m_mouse_info.pixel_coord.y - m_window_info.height / 2.0f;

    m_mouse_info.remapped_relative_to_center_coord.x =
        m_mouse_info.pixel_relative_to_center_coord.x / m_window_info.width * 2.0f;
    m_mouse_info.remapped_relative_to_center_coord.y =
        -m_mouse_info.pixel_relative_to_center_coord.y / m_window_info.height * 2.0f;
}

static glm::vec3 plane_normal{};
static uint32_t plane_normal_index;
static uint32_t gizmo_type_index;

void Application::update_gizmo()
{
    GizmoRaycastInfo gizmo_raycast =
        raycast_gizmos(m_mouse_info.remapped_relative_to_center_coord.x,
                       m_mouse_info.remapped_relative_to_center_coord.y, m_camera, m_scene.objects[m_focused_object]);

    if (gizmo_raycast.hit && !m_locked)
    {
        gizmo_type_index = static_cast<uint32_t>(gizmo_raycast.type);

        // NOTE: We need to chose from 2 normals because sometimes one of them is almost the same as camera forward
        // vector which gives artefacts

        uint32_t normal_index_1 = (gizmo_type_index + 1) % 3, normal_index_2 = (gizmo_type_index + 2) % 3;

        glm::vec3 normal_1{}, normal_2{};
        normal_1[normal_index_1] = 1.0f;
        normal_2[normal_index_2] = 1.0f;

        plane_normal *= 0.0f;

        if (glm::abs(glm::dot(m_camera.forward(), normal_1)) > glm::abs(glm::dot(m_camera.forward(), normal_2)))
        {
            plane_normal = normal_1;
            plane_normal_index = normal_index_1;
        }
        else
        {
            plane_normal = normal_2;
            plane_normal_index = normal_index_2;
        }

        m_locked = true;
    }
    if (m_locked)
    {
        ving::RaycastInfo raycast = ving::raycast_plane(
            m_mouse_info.remapped_relative_to_center_coord.x, m_mouse_info.remapped_relative_to_center_coord.y,
            m_camera, plane_normal, m_scene.objects[m_focused_object].transform.translation[plane_normal_index]);

        if (raycast.hit)
        {
            m_scene.objects[m_focused_object].transform.translation[gizmo_type_index] =
                raycast.position[gizmo_type_index] - ving::editor::Gizmo::length / 2.0f;
        }
    }
}
void Application::select_scene_object()
{
    if (!m_locked)
    {
        SceneRaycastInfo scene_raycast =
            ving::raycast_scene(m_mouse_info.remapped_relative_to_center_coord.x,
                                m_mouse_info.remapped_relative_to_center_coord.y, m_camera, m_scene);

        if (scene_raycast.hit)
        {
            m_focused_object = scene_raycast.object_id;
        }
    }
}
} // namespace ving
