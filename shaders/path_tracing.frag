#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 in_vpos;

layout (location = 0) out vec4 out_color;

const float infinite = 1.0 / 0.0;
// const float uint_max_float = 4294967295.0;
const uint uint_max = uint(0xffffffff);
const float uint_max_float = float(uint_max);

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
    float viewport_width;
    float viewport_height;
    uint time;
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

// Uniform
float random_float(inout uint seed)
{
    seed = pcg_hash(seed);
    return float(seed) / uint_max_float;
}
float random_float_normaldist(inout uint seed)
{
    float theta = 2 * 3.141252653 * random_float(seed);
    float rho = sqrt(-2 * log(random_float(seed)));
    return rho * cos(theta);
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
    // float x = random_float_normaldist(seed);
    // float y = random_float_normaldist(seed);
    // float z = random_float_normaldist(seed);
    float x = random_float(seed) * 2.0 - 1.0;
    float y = random_float(seed) * 2.0 - 1.0;
    float z = random_float(seed) * 2.0 - 1.0;

    return vec3(x, y, z);
    // return vec3(random_float(seed) * 2.0 - 1.0, random_float(seed) * 2.0 - 1.0, random_float(seed) * 2.0 - 1.0);
}
vec3 random_vec3_unit_sphere(inout uint seed)
{
    // vec3 p = random_vec3(seed);
    // while (length_squared(p) >= 1.0)
    // {
    //     p = random_vec3(seed);
    // }
    // return p;
    return normalize(random_vec3(seed));
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
        if (hit_sphere(spheres[i], ray, 0.01, infinite, record))
        {
            if (record.t < min_record.t)
            {
                min_record = record;
            }
            hit_anything = true;
        }
    }

    // if (hit_plane(plane, ray, 0.001, infinite, record))
    // {
    //     if (record.t < min_record.t)
    //     {
    //         min_record = record;
    //     }
    //     hit_anything = true;
    // }

    record = min_record;

    return hit_anything;
}

// Skybox
const float sun_radius = 0.2;
const vec3 sun_color = vec3(1.0, 1.0, 1.0);
// Anti aliasing
const int antialiasing_radius = 1;
const int samples_per_pixel = 5;

// Path tracing settings
const int max_bounces = 15;

const vec3 plane_normal = vec3(0.0, 1.0, 0.0);
const Plane scene_plane = Plane(normalize(plane_normal), 0.0, vec4(1.0, 1.0, 1.0, 1.0));

uint global_seed = 39818882;

void main()
{
    vec4 sampled_color = vec4(0.0);
    // uint seed = global_seed;

    uvec2 uifrag_coord = uvec2(gl_FragCoord);
    // uint seed = pcg_hash(uifrag_coord.x) + pcg_hash(uifrag_coord.y) + pcg_hash(pc.time);
    uint seed = pcg_hash(uifrag_coord.x) + pcg_hash(uifrag_coord.y);

    for (uint ray_sample = 0; ray_sample < samples_per_pixel; ++ray_sample)
    {
        // uint seed = ray_sample;
        float random_sample_x = (random_float(seed) * 2.0 - 1.0) * antialiasing_radius;
        float random_sample_y = (random_float(seed) * 2.0 - 1.0) * antialiasing_radius;
        
        vec2 frag_coord_0to1 = vec2((gl_FragCoord.x + random_sample_x) / pc.viewport_width, (gl_FragCoord.y + random_sample_y) / pc.viewport_height);
        // vec2 frag_coord = vec2(((gl_FragCoord.x + random_sample_x) / pc.viewport_width) * 2.0 - 1.0, ((gl_FragCoord.y + random_sample_y) / pc.viewport_height) * 2.0 - 1.0);
        vec2 frag_coord_min1to1 = frag_coord_0to1 * 2.0 - 1.0;
        vec3 center_quad_pos = frag_coord_min1to1.x * camera_info.right + -frag_coord_min1to1.y * camera_info.up;

        vec3 uvw = normalize(center_quad_pos + camera_info.forward);

        Ray ray = Ray(camera_info.position, uvw);
        HitRecord record;

        if (hit_closest_object_on_scene(ray, record, scene_plane))
        {
            vec4 ray_color = record.color;
            vec2 frag_coord_norm = normalize(frag_coord_0to1);

            // ray_color.xyz = (random_vec3_unit_sphere(seed) + 1.0) * 0.5;

            for (uint i = 0; i < max_bounces; ++i)
            {
                Ray bounce_ray = Ray(record.position, random_on_hemisphere(record.normal, seed));

                if (hit_closest_object_on_scene(bounce_ray, record, scene_plane))
                {
                    // ray_color += vec4(bounce_ray.direction, 1.0);
                    // ray_color *= 0.2;
                    ray_color *= 0.8 * record.color;
                }
                else
                {
                    break;
                }
            }
    
            // sampled_color += vec4((record.normal + 1.0) * 0.5, 1.0);
            sampled_color += ray_color;
            // sampled_color += 0.5 * vec4(random_on_hemisphere(record.normal, seed), 1.0) + 0.5;
        }
        else
        {
            // Skybox
            float sun_strength = clamp(0.0, 1.0, dot(uvw, scene_data.light_direction.xyz) - 1.0 + sun_radius) * scene_data.light_direction.w;
            vec4 skybox_color = texture(sampler_cube_map, uvw) + vec4(sun_color * sun_strength, 1.0);
            sampled_color += skybox_color;
        }
    }

    vec4 average_sample_color = sampled_color / samples_per_pixel;
    out_color = average_sample_color;
    
    // NOTE: To see random
    // vec2 frag_coord_0to1 = vec2(gl_FragCoord.x / pc.viewport_width, gl_FragCoord.y / pc.viewport_height);
    // vec2 frag_coord_norm = normalize(frag_coord_0to1);
    //
    // uvec2 uifrag_coord = uvec2(gl_FragCoord);
    // uint seed = pcg_hash(uifrag_coord.x) + pcg_hash(uifrag_coord.y);
    // out_color = vec4(random_on_hemisphere(vec3(1.0, 0.0, 0.0), seed), 1.0);

    // vec2 frag_coord = vec2((gl_FragCoord.x / pc.viewport_width) * 2.0 - 1.0, (gl_FragCoord.y / pc.viewport_height) * 2.0 - 1.0);
}
