#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 in_vpos;

layout (location = 0) out vec4 out_color;

const float infinite = 1.0 / 0.0;
// const float uint_max_float = 4294967295.0;
const float uint_max_float = float(uint(0xffffffff));

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
    int sphere_count;
    vec2 vertex_buffer; // WARN: Don't use it
    vec2 dummy;
} pc;

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
layout (set = 0, binding = 2) uniform CameraInfoBuffer
{
    CameraInfo camera_info;
};

struct SceneData
{
    vec4 light_direction;
};
layout (set = 0, binding = 3) uniform SceneDataBuffer
{
    SceneData scene_data;
};

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

float random_float(uint seed)
{
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
vec3 random_vec3(uint seed)
{
    float x = random_float(seed);
    seed = pcg_hash(seed);
    float y = random_float(seed);
    seed = pcg_hash(seed);
    float z = random_float(seed);
    seed = pcg_hash(seed);

    return vec3(x, y, z);
    // return vec3(random_float(seed) * 2.0 - 1.0, random_float(seed) * 2.0 - 1.0, random_float(seed) * 2.0 - 1.0);
}
vec3 random_vec3_unit_sphere(uint seed)
{
    vec3 p = random_vec3(seed);
    return normalize(p);
}
vec3 random_on_hemisphere(vec3 normal, uint seed)
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
const int max_bounces = 15;

const vec3 plane_normal = vec3(0.0, 1.0, 0.0);
const Plane scene_plane = Plane(normalize(plane_normal), 0.0, vec4(1.0, 1.0, 1.0, 1.0));

void main()
{

    // Ray position in camera space
    vec3 center_quad_pos = in_vpos.x * camera_info.right + -in_vpos.y * camera_info.up;
    vec3 rpos = camera_info.position + center_quad_pos;

    // vec3 rpos = in_vpos;
    // rpos.y *= -1;

    vec3 rdir = camera_info.forward;
    // vec3 rdir = vec3(0.0, 0.0, 1.0);

    Ray ray = Ray(rpos, rdir);

    vec3 uvw = normalize(center_quad_pos + ray.direction);

    // Skybox
    float sun_strength = clamp(0.0, 1.0, dot(uvw, scene_data.light_direction.xyz) - 1.0 + sun_radius) * scene_data.light_direction.w;

    HitRecord record;

    vec4 skybox_color = texture(sampler_cube_map, uvw) + vec4(sun_color * sun_strength, 1.0);

    if (hit_closest_object_on_scene(ray, record, scene_plane))
    {
        vec4 ray_color = record.color;

        vec2 vpos0to1 = (in_vpos.xy + 1.0) * 0.5;
        vpos0to1 = normalize(vpos0to1);

        // ray_color.xyz = (random_vec3_unit_sphere(seed) + 1.0) * 0.5;

        // for (int i = 0; i < max_bounces; ++i)
        // {
        //     uint seed = uint(vpos0to1.x * vpos0to1.y * uint_max_float);
        //
        //     Ray bounce_ray = Ray(vec3(record.position), random_on_hemisphere(record.normal, seed));
        //
        //     if (hit_closest_object_on_scene(bounce_ray, record, scene_plane))
        //     {
        //         ray_color *= 0.5;
        //     }
        // }

        out_color = ray_color;

        // NOTE: If you wanna see normals
        // out_color = vec4((record.normal + 1.0) * 0.5, 1.0);
    }
    else
    {
        out_color = skybox_color;
    }
}
