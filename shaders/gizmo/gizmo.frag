#version 460

layout (location = 0) flat in uint gizmo_direction;
layout (location = 0) out vec4 out_color;

const vec4 gizmo_direction_colors[4] = 
{
    vec4(1, 0, 0, 1),
    vec4(0, 1, 0, 1),
    vec4(0, 0, 1, 1),
	vec4(1, 1, 0, 1),
};

void main()
{
    out_color = gizmo_direction_colors[gizmo_direction];
}
