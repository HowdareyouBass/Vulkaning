#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec2 out_uv;

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
    float time;
    float delta_time;
} pc;

const float amplitude = 0.3;
const float wave_length = 500.0;
const float speed = 0.05;

void main()
{
    Vertex v = pc.vertex_buffer.vertices[gl_VertexIndex];

    float frequency = 2.0 / wave_length;
    float wave_speed = speed * 2.0 / wave_length;

    v.position.y += amplitude * sin(gl_VertexIndex * frequency + pc.time * wave_speed);

    gl_Position = pc.render_mtx * vec4(v.position, 1.0);

    out_color = v.color.xyz;
}
