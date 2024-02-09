#pragma once

#include <glm/mat4x4.hpp>

namespace ving
{
class PerspectiveCamera
{
  public:
    PerspectiveCamera(float aspect, float near, float far, float fov);

    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};

    float move_speed{0.01f};
    float look_speed{0.0002f};

    void update();

    [[nodiscard]] glm::mat4 projection() const noexcept { return m_projection; }
    [[nodiscard]] glm::mat4 view() const noexcept { return m_view; }

    [[nodiscard]] glm::vec3 forward() const noexcept { return m_forward; }
    [[nodiscard]] glm::vec3 right() const noexcept { return m_right; }
    [[nodiscard]] glm::vec3 up() const noexcept { return m_up; }

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
