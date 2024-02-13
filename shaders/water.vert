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
};

layout (set = 0, binding = 1) uniform SceneData
{
    UBObj ubobj;
};


void main()
{
    Vertex v = pc.vertex_buffer.vertices[gl_VertexIndex];

    float der_sum_x = 0.0;
    float der_sum_z = 0.0;
    
    for (int i = 0; i < pc.wave_count; ++i)
    {
        WaveData w = waves[i];

        float frequency = 2.0 / w.wlength;
        float wave_speed = w.speed * frequency;
        float wave_height = w.amplitude * sin(dot(w.direction, v.position.xz) * frequency + pc.time * wave_speed);

        float der_x = w.amplitude * frequency * w.direction.x * cos(dot(w.direction, v.position.xz) * frequency + pc.time * wave_speed);
        // NOTE: w.direction.y == w.direction.z because using 2d horizontal vector
        float der_z = w.amplitude * frequency * w.direction.y * cos(dot(w.direction, v.position.xz) * frequency + pc.time * wave_speed);

        der_sum_x += der_x;
        der_sum_z += der_z;

        v.position.y += wave_height;
    }

    // vec3 tangent = normalize(vec3(0.0, der_sum_x, 1.0));
    // vec3 binormal = normalize(vec3(1.0, der_sum_z, 0.0));
    vec3 tangent = vec3(1.0, der_sum_x, 0.0);
    vec3 binormal = vec3(0.0, der_sum_z, 1.0);

    v.normal = cross(binormal, tangent);
    // v.normal = cross(tangent, binormal);
    v.normal = normalize(v.normal);

    gl_Position = pc.render_mtx * vec4(v.position, 1.0);

    out_color = v.color.xyz;
    out_normal = v.normal;
    out_vpos = v.position;
}
