#version 450

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

void main()
{
    // out_color = frag_color;
    out_color = vec4(in_normal, 1.0);
}
