#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 in_vpos;

layout (location = 0) out vec4 out_color;

const float infinite = 1.0 / 0.0;

struct Sphere 
{
    vec3 center;
    float radius;
    vec4 color;
};
layout (set = 0, binding = 0) readonly buffer SphereBuffer
{
    Sphere spheres[];
};

layout (set = 0, binding = 1) uniform samplerCube sampler_cube_map;

layout (push_constant) uniform constants
{
    vec2 vertex_buffer; // WARN: Don't use it
    vec2 dummy;
    vec4 light_direction;
    int sphere_count;
} pc;


uint Hash(uint s)
{
    s ^= 2747636419u;
    s *= 2654435769u;
    s ^= s >> 16;
    s *= 2654435769u;
    s ^= s >> 16;
    s *= 2654435769u;
    return s;
}

float Random(uint seed)
{
    return float(Hash(seed)) / 4294967295.0; // 2^32-1
}


struct Ray
{
    vec3 position;
    vec3 direction;
};
struct HitRecord
{
    vec3 position;
    vec3 normal;
    vec4 color;
    float t;
    bool front_face;
};

float length_squared(vec3 vec)
{
    return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
}
vec3 random_vec3(uvec3 seed)
{
    return vec3(Random(seed.x), Random(seed.y), Random(seed.z));
}
vec3 random_vec3_unit_sphere(uvec3 seed)
{
    vec3 p = vec3(1.0, 1.0, 1.0);
    while (length_squared(p) >= 1)
    {
        p = random_vec3(seed);
        seed += 1;
    }
    return normalize(p);
}
vec3 random_on_hemisphere(vec3 normal, uvec3 seed)
{
    vec3 unit = random_vec3_unit_sphere(seed);
    if (dot(unit, normal) > 0.0)
    {
        return unit;
    }
    else
    {
        return -unit;
    }
}
vec3 ray_at(Ray ray, float t)
{
    return ray.position + ray.direction * t;
}

bool hit_sphere(Sphere s, Ray r, float min_t, float max_t , out HitRecord record)
{
    vec3 oc = r.position - s.center;
    float a = length_squared(r.direction);
    float half_b = dot(oc, r.direction);
    float c = length_squared(oc) - s.radius*s.radius;

    float discriminant = half_b*half_b - a*c;

    if (discriminant < 0)
    {
        return false; 
    }

    float t = (-half_b - sqrt(discriminant))/a;
    
    if (t < min_t || t > max_t)
    {
        t = (-half_b + sqrt(discriminant))/a;
    }
    if (t < min_t || t > max_t)
    {
        return false;
    }

    vec3 position = ray_at(r, t);
    vec3 normal = normalize(position - s.center);
    record = HitRecord(position, normal, s.color, t, (dot(r.direction, normal) < 0));
    return true;
}
// NOTE: Probably could do it smarter
bool hit_closest_sphere(Ray ray, out HitRecord record)
{
    HitRecord min_record;
    min_record.t = infinite;
    bool hit_anything = false;

    for (int i = 0; i < pc.sphere_count; ++i)
    {
        if (hit_sphere(spheres[i], ray, 0.001, infinite, record))
        {
            if (record.t < min_record.t)
            {
                min_record = record;
            }
            hit_anything = true;
        }
    }
    record = min_record;

    return hit_anything;
}

// Skybox
const float sun_radius = 0.2;
const vec3 sun_color = vec3(1.0, 1.0, 1.0);
// Anti aliasing
const int samples_per_pixel = 4;

// Path tracing settings
const int max_bounces = 2;

void main()
{
    Ray ray = Ray(in_vpos, vec3(0.0, 0.0, 1.0));
    ray.position.y *= -1;
    vec3 uvw = normalize(ray.position + ray.direction);
    // ray.position += camera_info.position;

    // Skybox
    float sun_strength = clamp(0.0, 1.0, dot(uvw, pc.light_direction.xyz) - 1.0 + sun_radius) * pc.light_direction.w;

    HitRecord record;

    if (hit_closest_sphere(ray, record))
    {
        vec4 ray_color = record.color;
        int num_bounces = 1;

        for (int i = 0; i < max_bounces; ++i)
        {
            uvec3 seed = ivec3(i * in_vpos * record.normal * 4294967295.0);

            Ray bounce_ray = Ray(vec3(record.position), vec3(random_on_hemisphere(record.normal, seed)));

            if (hit_closest_sphere(bounce_ray, record))
            {
                ray_color += record.color;
                ++num_bounces;
            }   
        }

        out_color = ray_color / num_bounces;
    }
    else
    {
        out_color = texture(sampler_cube_map, uvw) + vec4(sun_color * sun_strength, 1.0);
    }
}
