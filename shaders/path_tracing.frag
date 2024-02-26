#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 in_vpos;

layout (location = 0) out vec4 out_color;

const float infinite = 1.0 / 0.0;
const float uint_max_float = 4294967295.0;

struct Plane
{
    vec3 normal;
    float height;
    vec4 color;
};

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

uint pcg_hash(uint seed)
{
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float random_float(inout uint seed)
{
    seed = pcg_hash(seed); 
    return float(pcg_hash(seed)) / uint_max_float;
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
vec3 random_vec3(inout uint seed)
{
    return vec3(random_float(seed) * 2.0 - 1.0, random_float(seed) * 2.0 - 1.0, random_float(seed) * 2.0 - 1.0);
}
vec3 random_vec3_unit_sphere(inout uint seed)
{
    vec3 p = random_vec3(seed);
    return normalize(p);
}
vec3 random_on_hemisphere(vec3 normal, inout uint seed)
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
bool hit_plane(Plane p, Ray r, float min_t, float max_t, out HitRecord record)
{
    // float t = 1.0 / dot(p.normal, r.direction);
    // float t = dot(p.normal, r.direction);
    vec3 p0 = p.normal * p.height;

    float t = dot((p0 - r.position), p.normal)/dot(r.direction, p.normal);

    record = HitRecord(ray_at(r, t), p.normal, p.color, t, true);
   
    if (t < min_t)
    {
        record.t = infinite;
        return false;
    }
    else
    {
        return true;
    }

    return t >= min_t;
}
bool hit_closest_object_on_scene(Ray ray, out HitRecord record, Plane plane)
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

    if (hit_plane(plane, ray, 0.001, infinite, record))
    {
        if (record.t < min_record.t)
        {
            min_record = record;
        }
        hit_anything = true;
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
const int max_bounces = 10;

const vec3 plane_normal = vec3(0.0, 1.0, 0.0);
const Plane scene_plane = Plane(normalize(plane_normal), 0.0, vec4(1.0, 1.0, 1.0, 1.0));

void main()
{
    vec3 rpos = vec3(0.0, 0.0, 0.0);

    Ray ray = Ray(in_vpos, vec3(0.0, 0.0, 1.0));
    // Ray ray = Ray(rpos, vec3(gl_FragCoord.xy, -1.0));
    ray.position.y *= -1;
    vec3 uvw = normalize(ray.position + ray.direction);
    // vec3 uvw = ray.direction;
    // ray.position += camera_info.position;

    // Skybox
    float sun_strength = clamp(0.0, 1.0, dot(uvw, pc.light_direction.xyz) - 1.0 + sun_radius) * pc.light_direction.w;

    HitRecord record;

    vec4 skybox_color = texture(sampler_cube_map, uvw) + vec4(sun_color * sun_strength, 1.0);

    if (hit_closest_object_on_scene(ray, record, scene_plane))
    {
        vec4 ray_color = record.color;
        // int num_bounces = 1;
        //
        // for (int i = 0; i < max_bounces; ++i)
        // {
        //     uint seed = i * uint(gl_FragCoord.x * gl_FragCoord.y * uint_max_float);
        //
        //     Ray bounce_ray = Ray(vec3(record.position), vec3(random_on_hemisphere(record.normal, seed)));
        //
        //     if (hit_closest_object_on_scene(bounce_ray, record, scene_plane))
        //     {
        //         ray_color += record.color;
        //         ++num_bounces;
        //     }
        //     else
        //     {
        //         // ray_color += skybox_color;
        //         break;
        //     }
        // }
        //
        // out_color = ray_color / num_bounces;

        out_color = ray_color;

        // NOTE: If you wanna see normals
        // out_color = vec4((record.normal + 1.0) * 0.5, 1.0);
    }
    else
    {
        out_color = skybox_color;
    }
}
