#include "ving_camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ving
{
void Camera::set_perspective_projection()
{
    const float tanHalfFovy = tan(fov / 2.f);
    m_projection[0][0] = 1.f / (aspect * tanHalfFovy);
    // HACK: We negate the [1][1] because Vulkan has y axis pointed down
    m_projection[1][1] = -1.f / (tanHalfFovy);
    m_projection[2][2] = far / (far - near);
    m_projection[2][3] = 1.f;
    m_projection[3][2] = -(far * near) / (far - near);
}

void Camera::set_view_direction(glm::vec3 direction, glm::vec3 up)
{
    m_forward = glm::normalize(direction);
    m_right = glm::normalize(glm::cross(m_forward, up));
    m_up = glm::cross(m_forward, m_right);

    m_view = glm::mat4{1.f};
    m_view[0][0] = m_right.x;
    m_view[1][0] = m_right.y;
    m_view[2][0] = m_right.z;
    m_view[0][1] = m_up.x;
    m_view[1][1] = m_up.y;
    m_view[2][1] = m_up.z;
    m_view[0][2] = m_forward.x;
    m_view[1][2] = m_forward.y;
    m_view[2][2] = m_forward.z;
    m_view[3][0] = -glm::dot(m_right, position);
    m_view[3][1] = -glm::dot(m_up, position);
    m_view[3][2] = -glm::dot(m_forward, position);
}
} // namespace ving
