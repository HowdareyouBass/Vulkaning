#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

layout (location = 0) out vec4 out_color;

struct Vertex
{
    vec3 pos;
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
    int wave_count;
} pc;

struct WaveData
{
    float wave_length;
    float amplitude;
    float speed;
    float dummy;
    vec2 direction;
};
layout (set = 0, binding = 0) readonly buffer WavesBuffer
{
    WaveData waves[];
};

struct UBObj
{
    vec4 light_direction;
};
layout (set = 0, binding = 1) uniform SceneData
{
    UBObj ubobj;
};

void main()
{
    // out_color = vec4(frag_color.xyz, 1.0);
    out_color = vec4(frag_color * dot(ubobj.light_direction.xyz, normal), 1.0);
    // out_color = vec4(ubobj.light_direction.xyz, 1.0);
}
