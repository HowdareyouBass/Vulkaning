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
    int wave_count;
} pc;


// WARN: Length is a reserved word!!!
struct WaveData
{
    float wlength;
    float amplitude;
    float speed;
    vec2 direction;
};
layout (set = 0, binding = 0) readonly buffer WavesBuffer
{
    WaveData waves[];
};

// const float amplitude = 0.3;
// const float wave_length = 1.0;
// const float speed = 0.0005;
// const vec2 wave_direction = vec2(0.5, 0.5);

void main()
{
    Vertex v = pc.vertex_buffer.vertices[gl_VertexIndex];


    // for (int i = 0; i < pc.wave_count; ++i)
    // {
    //     WaveData w = waves[i];
    //
    //     float frequency = 2.0 / w.wlength;
    //     float wave_speed = w.speed * frequency;
    //     v.position.y += w.amplitude * sin(dot(w.direction, v.position.xz) * frequency + pc.time * wave_speed);
    // }

    WaveData w = waves[0];

    float frequency = 2.0 / w.wlength;
    float wave_speed = w.speed * frequency;
    v.position.y += w.amplitude * sin(dot(w.direction, v.position.xz) * frequency + pc.time * wave_speed);

    // float frequency = 2.0 / wave_length;
    // float wave_speed = speed * frequency;
    // v.position.y += amplitude * sin(dot(wave_direction, v.position.xz) * frequency + pc.time * wave_speed);

    gl_Position = pc.render_mtx * vec4(v.position, 1.0);

    out_color = v.color.xyz;
}
