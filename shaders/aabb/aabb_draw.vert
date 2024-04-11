#version 460

struct AABB
{
    float max_x;
    float max_y;
    float max_z;
    float min_x;
    float min_y;
    float min_z;
};

layout (push_constant) uniform constants
{
    mat4 pvm_transform;
    AABB aabb;
} pc;

// layout (set = 0, binding = 0) uniform AABBsVerticesBuffer
// {
//     vec4 aabb_positions[8];
// };

vec4 aabbs[8] = 
{
    vec4(pc.aabb.min_x, pc.aabb.min_y, pc.aabb.min_z, 1.0f),
    vec4(pc.aabb.min_x, pc.aabb.max_y, pc.aabb.min_z, 1.0f),
    vec4(pc.aabb.max_x, pc.aabb.max_y, pc.aabb.min_z, 1.0f),
    vec4(pc.aabb.max_x, pc.aabb.min_y, pc.aabb.min_z, 1.0f),

    vec4(pc.aabb.min_x, pc.aabb.min_y, pc.aabb.max_z, 1.0f),
    vec4(pc.aabb.min_x, pc.aabb.max_y, pc.aabb.max_z, 1.0f),
    vec4(pc.aabb.max_x, pc.aabb.max_y, pc.aabb.max_z, 1.0f),
    vec4(pc.aabb.max_x, pc.aabb.min_y, pc.aabb.max_z, 1.0f)
};

void main()
{
    gl_Position = pc.pvm_transform * aabbs[gl_VertexIndex];
} 
