#pragma once

#include <glm/mat4x4.hpp>

namespace ving
{
struct Camera
{
    void set_perspective_projection();
    void set_view_direction(glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});

    float aspect;
    float near;
    float far;
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    float fov;

    [[nodiscard]] glm::mat4 projection() const noexcept { return m_projection; }
    [[nodiscard]] glm::mat4 view() const noexcept { return m_view; }

    [[nodiscard]] glm::vec3 forward() const noexcept { return m_forward; }
    [[nodiscard]] glm::vec3 right() const noexcept { return m_right; }
    [[nodiscard]] glm::vec3 up() const noexcept { return m_up; }

  private:
    glm::mat4 m_projection{1.0f};
    glm::mat4 m_view{1.0f};

    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;
};
} // namespace ving
