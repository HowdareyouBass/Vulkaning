#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 out_color;

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
} pc;

void main()
{
    Vertex v = pc.vertex_buffer.vertices[gl_VertexIndex];

    gl_Position = pc.render_mtx * vec4(v.position, 1.0);

    out_color = v.color.xyz;
}
