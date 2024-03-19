#version 460

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec3 in_normal;
layout (location = 3) in vec3 in_vpos; // NOTE: World space vertex pos

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform constants
{
    mat4 model_transform;
    vec2 program_crash; // WARN: Don't use it
} pc;

struct SceneData
{
    vec4 light_direction;
    uint point_lights_count;
};
struct CameraInfo
{
    mat4 perspective_view_transform;
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
layout (binding = 1, set = 0) readonly buffer PointLightsBuffer
{
    PointLight point_lights[];
};

vec4 unlit(vec3 normal, vec3 view)
{
    return vec4(0.1, 0.1, 0.1, 1.0);
}

const vec4 light_color = vec4(1, 1, 1, 1);

const float epsilon = 0.0001;

void main()
{
    vec3 view = normalize(ubo.camera_info.position - in_vpos);
   
    float directional = clamp(dot(ubo.scene.light_direction.xyz, normalize(in_normal)), 0.0, 1.0) * ubo.scene.light_direction.w;

    float point = 0.0;

    for (int i = 0; i < ubo.scene.point_lights_count; ++i)
    {
        vec3 to_light = point_lights[i].position - vec3(pc.model_transform * vec4(in_vpos, 1.0));
        point += point_lights[i].intencity / (dot(to_light, to_light) + epsilon);
    }

    // out_color = unlit(in_normal, view) + (directional+point) * frag_color;
    out_color = frag_color + light_color * point;
    // out_color = vec4(0.5 * normalize(in_normal) + 0.5, 1.0);
}
