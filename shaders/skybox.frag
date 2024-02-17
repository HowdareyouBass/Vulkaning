#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 in_UVW;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform samplerCube sampler_cube_map;

layout (push_constant) uniform constants
{
    mat4 render_mtx;
    vec2 vertex_buffer; // WARN: DON"T USE IT!!!
    vec2 dummy;
    vec4 light_direction;
} pc;

struct CameraInfo
{
    vec3 up;
    float dummy;
    vec3 right;
    float dummy1;
    vec3 forward;
    float dummy2;
    vec3 position;
};

layout (set = 0, binding = 1) uniform CameraInfoBuffer
{
    CameraInfo camera_info;
};

void main()
{
    // out_color = vec4(frag_color, 1.0); 
    vec3 sun_color = vec3(0.9, 0.9, 0.9);

    float sun_strength = dot(camera_info.forward, pc.light_direction.xyz);

    out_color = texture(sampler_cube_map, in_UVW) + vec4(sun_color * sun_strength, 1.0);
}
