#pragma once

#include <SDL3/SDL_stdinc.h>

#include "ving_aabb_renderer.hpp"
#include "ving_gi_renderer.hpp"
#include "ving_gizmo_renderer.hpp"
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
    static constexpr bool test_rays = false;

  public:
    Application(SDL_Window *window);

    bool running() const { return m_running; }

    void run();
    void update();
    void stop();

  private:
    const Uint8 *keys;

    bool m_running{false};
    bool m_render_aabbs{false};

    SDL_Window *m_window;

    Core m_core;
    Scene m_scene;
    std::unordered_map<uint32_t, ving::Mesh> m_meshes;
    Profiler m_profiler;
    PerspectiveCamera m_camera;
    RenderFrames m_frames;

    ImGuiRenderer m_imgui_renderer;
    SkyboxRenderer m_skybox_renderer;
    GiRenderer m_gi_renderer;
    GizmoRenderer m_gizmo_renderer;
    AABBRenderer m_aabb_renderer;

    std::function<void()> m_render_aabbs_checkbox_imgui;
    std::function<void()> m_moving_scene_objects_imgui;
    std::function<void()> m_show_mouse_pos;
    std::function<void()> m_show_debug_bool;

    // NOTE: TEmporary
    uint32_t m_hit_id{0};
    bool m_debug_bool{false};
    bool m_first_hit{true};
    glm::vec2 m_first_hit_mouse_pos{0.0f, 0.0f};
};
} // namespace ving
