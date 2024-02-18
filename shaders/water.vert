#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec2 out_uv;
layout (location = 2) out vec3 out_normal;
layout (location = 3) out vec3 out_vpos;

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


// WARN: Length is a reserved word!!!
struct WaveData
{
    float wlength;
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
    vec3 viewer_pos;
};

layout (set = 0, binding = 1) uniform SceneData
{
    UBObj ubobj;
};

vec2 exp_sine_wave_derivative(WaveData w, Vertex v, float frequency, float wave_speed)
{
    return w.amplitude * frequency * w.direction * exp(w.amplitude * sin(dot(w.direction, v.position.xz) * frequency + pc.time * wave_speed) - 1) * cos(dot(w.direction, v.position.xz) * frequency + pc.time * wave_speed);
}

void main()
{
    Vertex v = pc.vertex_buffer.vertices[gl_VertexIndex];

    float der_sum_x = 0.0;
    float der_sum_z = 0.0;

    float der_prev_sum_x = 0.0;
    
    for (int i = 0; i < pc.wave_count; ++i)
    {
        WaveData w = waves[i];

        float frequency = 2.0 / w.wlength;
        float wave_speed = w.speed * frequency;
        // float wave_height = w.amplitude * sin(dot(w.direction, v.position.xz) * frequency + pc.time * wave_speed);
        float wave_height = exp(w.amplitude*sin(dot(w.direction, v.position.xz) * frequency + pc.time * wave_speed) - 1);

        vec2 derivative = exp_sine_wave_derivative(w, v, frequency, wave_speed);

        der_sum_x += derivative.x;
        der_sum_z += derivative.y;

        v.position.y += wave_height;

        int prev_index = gl_VertexIndex - 1;
        if (prev_index < 0)
        {
            prev_index = 0;
        }

        vec2 derivative_prev = exp_sine_wave_derivative(w, pc.vertex_buffer.vertices[prev_index], frequency, wave_speed);
        der_prev_sum_x += derivative_prev.x;
    }

    vec3 tangent = vec3(1.0, der_sum_x, 0.0);
    vec3 binormal = vec3(0.0, der_sum_z, 1.0);

    v.normal = cross(binormal, tangent);
    v.normal = normalize(v.normal);

    v.position.x += der_prev_sum_x;

    gl_Position = pc.render_mtx * vec4(v.position, 1.0);

    out_color = v.color.xyz;
    out_normal = v.normal;
    out_vpos = v.position;
}
