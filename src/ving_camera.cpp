#include "ving_camera.hpp"

namespace ving
{
void Camera::set_perspective_projection()
{
    const float aspect = width / height;
    const float tanHalfFovy = tan(fov / 2.f);
    m_projection[0][0] = 1.f / (aspect * tanHalfFovy);
    m_projection[1][1] = 1.f / (tanHalfFovy);
    m_projection[2][2] = far / (far - near);
    m_projection[2][3] = 1.f;
    m_projection[3][2] = -(far * near) / (far - near);
}

glm::mat4 Camera::projection() const noexcept
{
    return m_projection;
}
} // namespace ving
