#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec2 out_uv;

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
    mat4 pvm_transform;
    VertexBuffer vertex_buffer;
} pc;

void main()
{
    Vertex v = pc.vertex_buffer.vertices[gl_VertexIndex]; 

    gl_Position = pc.pvm_transform * vec4(v.position, 1.0);
    out_color = v.color;
    out_normal = v.normal;
    out_uv = vec2(v.uv_x, v.uv_y);
}
