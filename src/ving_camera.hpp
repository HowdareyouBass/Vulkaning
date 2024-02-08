#pragma once

#include <glm/mat4x4.hpp>

namespace ving
{
struct Camera
{
    void set_perspective_projection();
    void set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});

    float width;
    float height;
    float near;
    float far;
    float fov;

    [[nodiscard]] glm::mat4 projection() const noexcept { return m_projection; }
    [[nodiscard]] glm::mat4 view() const noexcept { return m_view; }

  private:
    glm::mat4 m_projection{1.0f};
    glm::mat4 m_view{1.0f};
};
} // namespace ving
