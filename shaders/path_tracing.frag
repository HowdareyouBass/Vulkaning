#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 in_UVW;
layout (location = 2) in vec3 in_vpos;

layout (location = 0) out vec4 out_color;

struct Sphere 
{
    vec3 center;
    float radius;
};
layout (set = 0, binding = 0) buffer SphereBuffer
{
    Sphere spheres[];
};

layout (set = 0, binding = 1) uniform samplerCube sampler_cube_map;

struct CameraInfo
{
    vec3 forward;
    float dummy0;
    vec3 up;
    float dummy1;
    vec3 right;
    float dummy2;
    vec3 position;
    float dummy3;
};
layout (set = 0, binding = 2) uniform CameraInfoUnifrom
{
    CameraInfo camera_info;
};

layout (push_constant) uniform constants
{
    vec2 vertex_buffer; // WARN: Don't use it
    vec2 dummy;
    vec4 light_direction;
    int sphere_count;
} pc;

const float sun_radius = 0.2;
const vec3 sun_color = vec3(1.0, 1.0, 1.0);

struct Ray
{
    vec3 position;
    vec3 direction;
};
bool hit_sphere(Sphere s, Ray r)
{
    float a = dot(r.direction, r.direction);
    float b = 2*dot(r.direction, r.position - s.center);
    float c = dot(r.position - s.center, r.position - s.center) - s.radius*s.radius;

    float discriminant = b*b - 4*a*c;

    return (discriminant >= 0);
}

void main()
{
    // out_color = vec4(frag_color, 1.0); 
    // vec3 sun_color = vec3(0.9, 0.9, 0.9);

    vec3 uvw = normalize(in_UVW);
    uvw.z *= -1;

    float sun_strength = clamp(0.0, 1.0, dot(uvw, pc.light_direction.xyz) - 1.0 + sun_radius) * pc.light_direction.w;

    for (int i = 0; i < pc.sphere_count; ++i)
    {
        out_color = texture(sampler_cube_map, in_UVW) + vec4(sun_color * sun_strength, 1.0);
        Ray r = Ray(vec3(in_vpos.xy, 0.0), normalize(camera_info.forward));
        // Ray r = Ray(vec3(in_vpos.xy, 0.0), vec3(0.0, 0.0, 1.0));
        if (hit_sphere(spheres[i], r))
        {
            out_color = vec4(1.0, 0.0, 0.0, 1.0); 
        }
    }

}
