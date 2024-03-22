#version 460

layout (location = 0) flat in uint in_vidx;

layout (location = 0) out vec4 out_color;

void main()
{
    // out_color = vec4(1.0, 1.0, 0.0, 1.0);
    out_color = vec4(0.1, 0.1, 0.1, 1.0) * in_vidx;
}
