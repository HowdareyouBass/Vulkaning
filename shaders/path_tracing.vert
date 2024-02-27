#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec3 out_vpos;

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
    int sphere_count;
    VertexBuffer vertex_buffer;
    vec2 dummy;
} pc;

struct CameraInfo
{
    vec3 forward;
    float dummy0;
    vec3 up;
    float dummy1;
    vec3 right;
    float dummy2;
    vec3 position;
    float dummy3;
};
layout (set = 0, binding = 2) uniform CameraInfoUniform
{
    CameraInfo camera_info;
};

const float sun_radius = 0.2;
const vec3 sun_color = vec3(1.0, 1.0, 1.0);

void main()
{
    Vertex v = pc.vertex_buffer.vertices[gl_VertexIndex];

    // out_UVW = normalize(v.position.x * camera_info.right + -v.position.y * camera_info.up + camera_info.forward);
    out_vpos = v.position;

    gl_Position = vec4(v.position, 1.0);
}
