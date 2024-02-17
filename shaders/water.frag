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
const float specular_size = 200.0;

void main()
{
    vec3 view = normalize(ubobj.viewer_position - in_vpos);
    // vec3 view = normalize(in_vpos - ubobj.viewer_position);
    vec3 half_way = normalize(view + ubobj.light_direction.xyz);

    // float specular = pow(dot(half_way, in_normal), specular_size);
    // float diffuse = dot(ubobj.light_direction.xyz, in_normal);

    float sum_der_x = 0;
    float sum_der_z = 0;

    for (int i = 0; i < pc.wave_count; ++i)
    {
        WaveData w = waves[i];
        float frequency = 2.0 / w.wave_length;
        float wave_speed= w.speed * frequency;

        // float der_x = w.amplitude * frequency  * w.direction.x * cos(dot(w.direction, in_vpos.xz) * frequency + pc.time * wave_speed);
        // float der_z = w.amplitude * frequency  * w.direction.y * cos(dot(w.direction, in_vpos.xz) * frequency + pc.time * wave_speed);

        vec2 derivative = w.amplitude * frequency * w.direction * exp(w.amplitude * sin(dot(w.direction, in_vpos.xz) * frequency + pc.time * wave_speed) - 1) * cos(dot(w.direction, in_vpos.xz) * frequency + pc.time * wave_speed);

        sum_der_x += derivative.x;
        sum_der_z += derivative.y;
    }

    vec3 tangent = vec3(1.0, sum_der_x, 0.0);
    vec3 binormal = vec3(0.0, sum_der_x, 1.0);

    vec3 normal = normalize(cross(binormal, tangent));

    float specular = pow(dot(half_way, normal), specular_size);
    float diffuse = dot(ubobj.light_direction.xyz, normal);

    out_color = vec4(frag_color * diffuse + specular * specular_color, 1.0);
    // out_color = vec4(ubobj.viewer_position, 1.0);
}
