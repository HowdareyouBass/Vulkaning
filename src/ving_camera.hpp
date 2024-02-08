#pragma once

#include <glm/mat4x4.hpp>

namespace ving
{
struct Camera
{
    void set_perspective_projection();

    float width;
    float height;
    float near;
    float far;
    float fov;

    [[nodiscard]] glm::mat4 projection() const noexcept;

  private:
    glm::mat4 m_projection;
};
} // namespace ving
