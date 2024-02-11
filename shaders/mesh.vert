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
} PushConstants;

// layout (set = 0, binding = 0) uniform UniformBufferObject
// {
//     vec3 light_dir;
// } ubo;

struct Ubobj
{
    vec4 light_dir;
};
layout (std140, set = 0, binding = 0) readonly buffer SceneDataBuffer
{
    Ubobj obj;
};

void main()
{
    Vertex v = PushConstants.vertex_buffer.vertices[gl_VertexIndex];

    gl_Position = PushConstants.render_mtx * vec4(v.position, 1.0);

    // out_color = v.color.xyz * max(0.1, dot(v.normal, normalize(obj.light_dir.xyz)));
    out_color=  v.color.xyz;
    out_uv.x = v.uv_x;
    out_uv.y = v.uv_y;
}
