#version 460

layout (location = 0) out uint out_vidx;

layout (push_constant) uniform constants
{
    mat4 pvm_transform;
} pc;

layout (set = 0, binding = 0) uniform AABBsVerticesBuffer
{
    vec4 aabb_positions[8];
};

void main()
{
    gl_Position = pc.pvm_transform * aabb_positions[gl_VertexIndex];
    // gl_Position = vec4(aabb_positions[gl_VertexIndex], 1.0);

    out_vidx = gl_VertexIndex;
} 
