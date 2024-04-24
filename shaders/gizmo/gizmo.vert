#version 460
#extension GL_EXT_buffer_reference : require

layout (location = 0) flat out uint out_gizmo_direction;

layout (push_constant) uniform Constants
{
    mat4 perspective_view_transform;
    vec3 object_position;
} pc;

const vec3 offsets[6] =
{
    vec3(0, 0, 0), 
    vec3(1, 0, 0), 
    vec3(0, 0, 0), 
    vec3(0, 1, 0), 
    vec3(0, 0, 0), 
    vec3(0, 0, 1) 
};

const float line_length = 0.5;

void main()
{
    gl_Position = pc.perspective_view_transform * vec4(pc.object_position + offsets[gl_VertexIndex] * line_length, 1.0);

    // floor(gl_VertexIndex / 2.0);

    out_gizmo_direction = uint(floor(gl_VertexIndex / 2.0));
}
