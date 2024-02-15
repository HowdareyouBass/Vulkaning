#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_vpos;

layout (location = 0) out vec4 out_color;

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
    vec3 viewer_position;
};
layout (set = 1, binding = 0) uniform SceneData
{
    UBObj ubobj;
};

const vec3 specular_color = vec3(1.0, 1.0, 1.0);
const float specular_size = 5.0;

void main()
{
    vec3 view = normalize(ubobj.viewer_position - in_vpos);
    vec3 half_way = normalize(view + ubobj.light_direction.xyz);

    float specular = pow(dot(half_way, in_normal), specular_size);
    float diffuse = dot(ubobj.light_direction.xyz, in_normal);

    out_color = vec4(frag_color * diffuse + specular * specular_color, 1.0);
    // out_color = vec4(ubobj.viewer_position, 1.0);
}
