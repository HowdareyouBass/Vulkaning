#include "ving_camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

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
    m_projection[1][1] = 1.0f / tan_half_fov;
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
    // const float c3 = glm::cos(rotation.z);
    // const float s3 = glm::sin(rotation.z);
    // const float c2 = glm::cos(rotation.x);
    // const float s2 = glm::sin(rotation.x);
    // const float c1 = glm::cos(rotation.y);
    // const float s1 = glm::sin(rotation.y);
    // m_right = glm::vec3{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    // m_up = glm::vec3{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    // m_forward = glm::vec3{(c2 * s1), (-s2), (c1 * c2)};
    // m_view = glm::mat4{1.f};
    // m_view[0][0] = m_right.x;
    // m_view[1][0] = m_right.y;
    // m_view[2][0] = m_right.z;
    // m_view[0][1] = m_up.x;
    // m_view[1][1] = m_up.y;
    // m_view[2][1] = m_up.z;
    // m_view[0][2] = m_forward.x;
    // m_view[1][2] = m_forward.y;
    // m_view[2][2] = m_forward.z;
    // m_view[3][0] = -glm::dot(m_right, position);
    // m_view[3][1] = glm::dot(m_up, position);
    // m_view[3][2] = -glm::dot(m_forward, position);

    m_view = glm::mat4{1.0f};

    // NOTE: Negating y component to convert from vulkan space to engine space
    glm::vec3 engine_space_pos = position;
    engine_space_pos.y = -engine_space_pos.y;

    // glm::mat4 translation_transform{1.0f};
    //
    // translation_transform[3][0] = -glm::dot(m_right, engine_space_pos);
    // translation_transform[3][1] = -glm::dot(m_up, engine_space_pos);
    // translation_transform[3][2] = -glm::dot(m_forward, engine_space_pos);

    // glm::mat4 rotation_transform{1.0f};
    // rotation_transform = glm::rotate(rotation_transform, -rotation.y, {0.0f, 1.0f, 0.0f}) *
    //                      glm::rotate(rotation_transform, -rotation.x, {1.0f, 0.0f, 0.0f}) *
    //                      glm::rotate(rotation_transform, -rotation.z, {0.0f, 0.0f, 1.0f});

    const float s1 = glm::sin(rotation.y);
    const float c1 = glm::cos(rotation.y);
    const float s2 = glm::sin(rotation.x);
    const float c2 = glm::cos(rotation.x);
    const float s3 = glm::sin(rotation.z);
    const float c3 = glm::cos(rotation.z);

    // m_right = {c1 * c3 + s1 * s2 * s3, c2 * s3, c1 * s2 * s3 - c3 * s1};
    // m_up = {c3 * s1 * s2 - c1 * s3, c2 * c3, c1 * c3 * s2 + s1 * s3};
    // m_forward = {c2 * s1, -s2, c1 * c2};

    m_right = glm::vec3{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    m_up = glm::vec3{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    m_forward = glm::vec3{(c2 * s1), (-s2), (c1 * c2)};

    // glm::mat4 rotation_transform = {
    //     {m_right, 0.0f},
    //     {m_up, 0.0f},
    //     {m_forward, 0.0f},
    //     {0.0f, 0.0f, 0.0f, 1.0f},
    // };
    // rotation_transform = glm::transpose(rotation_transform);
    //
    // m_view = rotation_transform * translation_transform;

    m_view[0][0] = m_right.x;
    m_view[1][0] = m_right.y;
    m_view[2][0] = m_right.z;
    m_view[0][1] = m_up.x;
    m_view[1][1] = m_up.y;
    m_view[2][1] = m_up.z;
    m_view[0][2] = m_forward.x;
    m_view[1][2] = m_forward.y;
    m_view[2][2] = m_forward.z;

    m_view[3][0] = -glm::dot(m_right, engine_space_pos);
    m_view[3][1] = -glm::dot(m_up, engine_space_pos);
    m_view[3][2] = -glm::dot(m_forward, engine_space_pos);
}
} // namespace ving
