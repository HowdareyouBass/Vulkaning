#version 460

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec3 in_normal;
layout (location = 3) in vec3 in_vpos;

layout (location = 0) out vec4 out_color;

// layout (push_constant) uniform PushCosntants
// {
//     mat4 pvm_transform;
//     vec2 segfault;
// } pushc;

struct SceneData
{
    vec4 light_direction;
    uint point_lights_count;
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
    float dummy3;
};
struct UniformBufferObject
{
    CameraInfo camera_info;
    SceneData scene;
};
layout (binding = 0, set = 0) uniform Ubo
{
    UniformBufferObject ubo;
};

struct PointLight
{
    vec3 position;
    float intencity;
    vec4 color;
};

// layout (binding = 1, set = 0) uniform PointLightsBuffer
// {
//     PointLight point_lights[];
// };

vec4 unlit(vec3 normal, vec3 view)
{
    return vec4(0.1, 0.1, 0.1, 1.0);
}

void main()
{
    vec3 view = normalize(ubo.camera_info.position - in_vpos);
   
    float directional = clamp(dot(ubo.scene.light_direction.xyz, in_normal), 0.0, 1.0) * ubo.scene.light_direction.w;

    float pp = 0.0;

    for (int i = 0; i < ubo.scene.point_lights_count; ++i)
    {
        pp += 1.0;
    }

    // out_color = unlit(in_normal, view) + (directional + pp) * frag_color;
    out_color = unlit(in_normal, view) + (directional+pp) * frag_color;
}
