#version 460

layout (location = 0) in vec4 frag_color;
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

vec4 unlit(vec3 normal, vec3 view)
{
    return vec4(0.1, 0.1, 0.1, 1.0);
}

void main()
{
    vec3 view = normalize(ubo.camera_info.position - in_vpos);
    
    out_color = unlit(in_normal, view) + clamp(dot(ubo.scene.light_direction.xyz, in_normal), 0.0, 1.0) * frag_color;
}
