#pragma once

#include <glm/mat4x4.hpp>

namespace ving
{
// Shader aligned camera information
struct CameraInfo
{
    glm::vec3 forward;
    float dummy0;
    glm::vec3 up;
    float dummy1;
    glm::vec3 right;
    float dummy2;
    glm::vec3 position;
    float dummy3;
};

class PerspectiveCamera
{
  public:
    PerspectiveCamera(float aspect, float near, float far, float fov);

    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};

    float move_speed{0.01f};
    float mouse_look_speed{0.0002f};
    float arrows_look_speed{0.001f};

    void update();

    [[nodiscard]] glm::mat4 projection() const noexcept { return m_projection; }
    [[nodiscard]] glm::mat4 view() const noexcept { return m_view; }

    [[nodiscard]] glm::vec3 forward() const noexcept { return m_forward; }
    [[nodiscard]] glm::vec3 right() const noexcept { return m_right; }
    [[nodiscard]] glm::vec3 up() const noexcept { return m_up; }

    [[nodiscard]] CameraInfo camera_info() const noexcept
    {
        return CameraInfo{m_forward, 0.0f, m_up, 0.0f, m_right, 0.0f, position, 0.0f};
    }

  private:
    void set_view_direction(glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});

    void update_projection();
    void update_view();

    float m_aspect;
    float m_near;
    float m_far;
    float m_fov;

    glm::mat4 m_projection{1.0f};
    glm::mat4 m_view{1.0f};

    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;
};
} // namespace ving
