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
layout (set = 0, binding = 1) uniform SceneData
{
    UBObj ubobj;
};
layout (set = 1, binding = 0) uniform samplerCube skybox;

const vec3 specular_color = vec3(1.0, 1.0, 1.0);
const float specular_size = 70.0;

void main()
{
    vec3 to_viewer = normalize(ubobj.viewer_position - in_vpos);
    vec3 half_way = normalize(to_viewer + ubobj.light_direction.xyz);

    float sum_der_x = 0;
    float sum_der_z = 0;

    for (int i = 0; i < pc.wave_count; ++i)
    {
        WaveData w = waves[i];
        float frequency = 2.0 / w.wave_length;
        float wave_speed= w.speed * frequency;

        vec2 derivative = w.amplitude * frequency * w.direction * exp(w.amplitude * sin(dot(w.direction, in_vpos.xz) * frequency + pc.time * wave_speed) - 1) * cos(dot(w.direction, in_vpos.xz) * frequency + pc.time * wave_speed);

        sum_der_x += derivative.x;
        sum_der_z += derivative.y;
    }

    vec3 tangent = normalize(vec3(1.0, sum_der_x, 0.0));
    vec3 binormal = normalize(vec3(0.0, sum_der_z, 1.0));

    vec3 normal = cross(binormal, tangent);

    float specular = pow(dot(half_way, normal), specular_size);
    float diffuse = clamp(0, 1, dot(ubobj.light_direction.xyz, normal));

    vec3 from_viewer = to_viewer;
    vec3 reflect = 2.0 * (normal * dot(normal, from_viewer)) - from_viewer;
    vec4 skybox = texture(skybox, reflect);

    float fresnel = pow(1.0 - dot(to_viewer, normal), 3);

    // out_color = vec4(frag_color * diffuse + specular * specular_color, 1.0);
    out_color = skybox * fresnel;
    out_color = vec4(frag_color * diffuse * (1.0 - fresnel), 1.0) + vec4(specular * specular_color * fresnel, 1.0) + skybox * fresnel;
}
