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

void Camera::set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    const glm::vec3 w{glm::normalize(direction)};
    const glm::vec3 u{glm::normalize(glm::cross(w, up))};
    const glm::vec3 v{glm::cross(w, u)};

    m_view = glm::mat4{1.f};
    m_view[0][0] = u.x;
    m_view[1][0] = u.y;
    m_view[2][0] = u.z;
    m_view[0][1] = v.x;
    m_view[1][1] = v.y;
    m_view[2][1] = v.z;
    m_view[0][2] = w.x;
    m_view[1][2] = w.y;
    m_view[2][2] = w.z;
    m_view[3][0] = -glm::dot(u, position);
    m_view[3][1] = -glm::dot(v, position);
    m_view[3][2] = -glm::dot(w, position);
}
} // namespace ving
