#version 460

layout (location = 1) in vec3 in_normal;
layout (location = 3) in vec3 in_vpos;

layout (location = 0) out vec4 out_color;

const vec4 cool_color = vec4(0, 0, 0.55, 1);
const vec4 warm_color = vec4(0.3, 0.3, 0, 1);
const vec4 highlight_color = vec4(1, 1, 1, 1);

struct SceneData
{
    vec4 light_direction;
};
struct CameraInfo
{
    vec3 forward;
    float dummy;
    vec3 up;
    float dummy1;
    vec3 right;
    float dummy2;
    vec3 position;
};
struct UniformBufferObject
{
    SceneData scene;
    CameraInfo camera_info;
};
layout (binding = 0, set = 0) uniform Ubo
{
    UniformBufferObject ubo;
};

void main()
{
    vec3 view = normalize(ubo.camera_info.position - in_vpos);
    vec3 reflect = 2.0 * dot(in_normal, ubo.scene.light_direction.xyz) * in_normal - ubo.scene.light_direction.xyz;
    float specular = clamp(100 * dot(reflect, view) - 97, 0.0, 1.0); 
    float t = (dot(in_normal, ubo.scene.light_direction.xyz) + 1.0) / 2.0;

    out_color = specular * highlight_color + (1 - specular) * (t * warm_color + (1 - t) * cool_color);
}
