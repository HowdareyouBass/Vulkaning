#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 inUVW;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform samplerCube sampler_cube_map;

void main()
{
    // out_color = vec4(frag_color, 1.0); 
    out_color = texture(sampler_cube_map, inUVW);
}
