#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec3 out_UVW;

struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
};

layout (buffer_reference, std430) readonly buffer VertexBuffer
{
    Vertex vertices[];
};

layout (push_constant) uniform constants
{
    mat4 render_mtx;
    VertexBuffer vertex_buffer;
    vec2 dummy;
    vec4 lightning_direction;
} pc;

layout (set = 0, binding = 0) uniform samplerCube sampler_cube_map;

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

    Vertex v = pc.vertex_buffer.vertices[gl_VertexIndex];

    // out_UVW = normalize(vec3(v.position.xy, 1.0));
    // out_UVW = vec3(pc.camera_forward.xy + v.position.xy, 1.0);
    // out_UVW = normalize(pc.camera_forward + v.position);
    out_UVW = normalize(v.position.x * camera_info.right + -v.position.y * camera_info.up + camera_info.forward);
    // out_UVW.yz *= -1.0;

    gl_Position = pc.render_mtx * vec4(v.position, 1.0);

    // out_color = v.color.xyz;
}
