#pragma once

#include <SDL3/SDL_stdinc.h>

#include "ving_aabb_renderer.hpp"
#include "ving_gi_renderer.hpp"
#include "ving_gizmo_renderer.hpp"
#include "ving_gizmos.hpp"
#include "ving_imgui_renderer.hpp"
#include "ving_profilers.hpp"
#include "ving_scene.hpp"
#include "ving_skybox_renderer.hpp"

namespace ving
{
class Application
{
  public:
    static constexpr float fov = 60.0f;

    struct MouseInfo
    {
        Uint32 buttons;
        glm::vec2 pixel_coord;
        glm::vec2 pixel_relative_to_center_coord;
        glm::vec2 remapped_relative_to_center_coord;
    };
    struct WindowInfo
    {
        float width;
        float height;
    };

  public:
    explicit Application(SDL_Window *window);

    bool running() const { return m_running; }

    void run();
    void update();
    void stop();

  private:
    void update_mouse_info();
    void update_gizmo();
    void select_scene_object();
    float get_gizmo_length();

  private:
    const Uint8 *keys = nullptr;

    bool m_running{false};
    bool m_render_aabbs{false};

    SDL_Window *m_window = nullptr;

    Core m_core;
    Scene m_scene;
    // Maybe should use shared_ptr because if this destroys anything using the meshes will segfault
    std::unordered_map<uint32_t, ving::Mesh> m_meshes;
    Profiler m_profiler;
    PerspectiveCamera m_camera;
    RenderFrames m_frames;

    ImGuiRenderer m_imgui_renderer;
    SkyboxRenderer m_skybox_renderer;
    GiRenderer m_gi_renderer;
    GizmoRenderer m_gizmo_renderer;
    AABBRenderer m_aabb_renderer;

    MouseInfo m_mouse_info;
    WindowInfo m_window_info;

    std::function<void()> m_render_aabbs_checkbox_imgui;
    std::function<void()> m_moving_scene_objects_imgui;
    std::function<void()> m_focused_object_transform;
    std::function<void()> m_show_mouse_pos;
    std::function<void()> m_show_debug_bool;
    std::function<void()> m_show_log_string;

    uint32_t m_focused_object{0};
    bool m_locked{false};
    // NOTE: TEmporary
    ving::editor::Gizmo::Type m_gizmo_type;
    bool m_debug_bool{false};
    bool m_first_hit{true};
    std::string m_log_string{};
    glm::vec2 m_first_hit_mouse_pos{0.0f, 0.0f};
    uint32_t m_gizmo_type_index;

    enum MeshType
    {
        Plane = 0,
        SmoothVase,
        Cube,
        Dragon,
    };
};
} // namespace ving
