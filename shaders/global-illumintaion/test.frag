#version 450

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

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
    // out_color = frag_color;
    // out_color = vec4(in_normal * 0.5 + 0.5, 1.0);
    
    float diffuse = dot(in_normal, ubo.scene.light_direction.xyz);

    out_color = diffuse * frag_color;
}
