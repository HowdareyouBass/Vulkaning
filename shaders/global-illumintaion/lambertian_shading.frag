#version 460
#extension GL_EXT_buffer_reference : enable

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec3 in_normal;
layout (location = 3) in vec3 in_vpos;

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform PushCosntants
{
    mat4 pvm_transform;
    vec2 segfault;
    uint pcount;
} pc;

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
   
    float directional = clamp(dot(ubo.scene.light_direction.xyz, in_normal), 0.0, 1.0);

    float p = 0.0;

    uint a = 0;
    a += pc.pcount;

    for (int i = 0; i < a; ++i)
    {
        p += 0.01;
    }


    out_color = unlit(in_normal, view) + (directional + p) * frag_color;
}
