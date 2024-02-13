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
};
layout (set = 0, binding = 1) uniform SceneData
{
    UBObj ubobj;
};

const vec3 lightdir = vec3(-1.0, -0.5, -1.0);

void main()
{
    // float der_sum_x = 0.0;
    // float der_sum_z = 0.0;
    //
    // for (int i = 0; i < pc.wave_count; ++i)
    // {
    //     WaveData w = waves[i];
    //
    //     float frequency = 2.0 / w.wave_length;
    //     float wave_speed = frequency * w.speed;
    //
    //     float der_x = w.amplitude * frequency * w.direction.x * cos(dot(w.direction, in_vpos.xz) * frequency + pc.time * wave_speed);
    //     float der_z = w.amplitude * frequency * w.direction.y * cos(dot(w.direction, in_vpos.xz) * frequency + pc.time * wave_speed);
    //
    //     der_sum_x += der_x;
    //     der_sum_z += der_z;
    // }
    //
    // vec3 tangent = normalize(vec3(1.0, der_sum_x, 0.0));
    // vec3 binormal = normalize(vec3(0.0, der_sum_z, 1.0));
    //
    // vec3 norm = normalize(cross(binormal, tangent));

    // out_color = vec4(frag_color.xyz, 1.0);
    // out_color = vec4(frag_color * max(0.1, dot(normalize(lightdir), norm)), 1.0);
    // out_color = vec4(norm, 1.0);
    out_color = vec4(in_normal, 1.0);
    out_color = vec4(frag_color * dot(ubobj.light_direction.xyz, in_normal), 1.0);
    // out_color = vec4(ubobj.light_direction.xyz, 1.0);
}
