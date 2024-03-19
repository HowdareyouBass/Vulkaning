#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec2 out_uv;
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
    mat4 model_transform;
    VertexBuffer vertex_buffer;
} pc;

struct SceneData
{
    vec4 light_direction;
    uint point_lights_count;
};
struct CameraInfo
{
    mat4 perspective_view_transform;
    vec3 forward;
    float dummy;
    vec3 up;
    float dummy1;
    vec3 right;
    float dummy2;
    vec3 position;
    float dummy3;
};
struct UniformBufferObject
{
    CameraInfo camera_info;
    SceneData scene;
};

layout (binding = 0, set = 0) uniform Ubo
{
    UniformBufferObject ubo;
};
struct PointLight
{
    vec3 position;
    float intencity;
    vec4 color;
};
layout (binding = 1, set = 0) readonly buffer PointLightsBuffer
{
    PointLight point_lights[];
};

const float epsilon = 0.0001;

void main()
{
    Vertex v = pc.vertex_buffer.vertices[gl_VertexIndex]; 

    gl_Position = ubo.camera_info.perspective_view_transform * pc.model_transform * vec4(v.position, 1.0);
    out_normal = v.normal;
    out_uv = vec2(v.uv_x, v.uv_y);
    out_color = v.color;
    // out_vpos = vec3(vec4(v.position, 1.0) * pc.model_transform);
    out_vpos = v.position;
}
