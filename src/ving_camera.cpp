#include "ving_camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace ving
{
PerspectiveCamera::PerspectiveCamera(float aspect, float near, float far, float fov)
    : m_aspect{aspect}, m_near{near}, m_far{far}, m_fov{fov}
{
    update_projection();
}
void PerspectiveCamera::update()
{
    update_view();
}
void PerspectiveCamera::update_projection()
{
    const float tan_half_fov = tan(m_fov / 2.0f);

    m_projection[0][0] = 1.0f / (m_aspect * tan_half_fov);
    m_projection[1][1] = -1.0f / tan_half_fov;
    m_projection[2][2] = m_far / (m_far - m_near);
    m_projection[3][2] = -m_far * m_near / (m_far - m_near);
    m_projection[2][3] = 1.0f;
    m_projection[3][3] = 0.0f;
}

void PerspectiveCamera::set_view_direction(glm::vec3 direction, glm::vec3 up)
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
void PerspectiveCamera::update_view()
{
    m_view = glm::mat4{1.0f};

    glm::mat4 rot{1.0f};
    rot = glm::rotate(rot, rotation.y, {0.0f, 1.0f, 0.0f});
    rot = glm::rotate(rot, rotation.x, {1.0f, 0.0f, 0.0f});
    rot = glm::rotate(rot, rotation.z, {0.0f, 0.0f, 1.0f});
    m_right = rot[0];
    m_up = rot[1];
    m_forward = rot[2];
    rot = glm::transpose(rot);

    glm::mat4 trans = glm::translate(-position);

    m_view = rot * trans;
}
} // namespace ving
