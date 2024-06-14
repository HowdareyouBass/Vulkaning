#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "debug/ving_logger.hpp"
#include "ving_core.hpp"
#include "ving_mesh.hpp"
#include "ving_vertex.hpp"

namespace ving
{
Mesh load_from_file_obj(const Core &core, std::string_view filepath, glm::vec4 color)
{
    tinyobj::attrib_t attrib;
    AABB aabb;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err, warn;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.data());

    if (!warn.empty())
        std::cout << warn << std::endl;
    if (!err.empty())
        std::cout << err << std::endl;
    if (!ret)
        throw std::runtime_error("Failed to load model");

    std::vector<Vertex> model_vertices{};
    model_vertices.reserve(attrib.vertices.size() / 3);
    std::vector<uint32_t> model_indices{};

    for (size_t i = 0; i < attrib.vertices.size(); i += 3)
    {
        model_vertices.push_back(
            Vertex{{attrib.vertices[i + 0], attrib.vertices[i + 1], attrib.vertices[i + 2]}, 0, {}, 0, color});

        aabb.max.x = std::max(aabb.max.x, model_vertices.back().position.x);
        aabb.min.x = std::min(aabb.min.x, model_vertices.back().position.x);
        aabb.max.y = std::max(aabb.max.y, model_vertices.back().position.y);
        aabb.min.y = std::min(aabb.min.y, model_vertices.back().position.y);
        aabb.max.z = std::max(aabb.max.z, model_vertices.back().position.z);
        aabb.min.z = std::min(aabb.min.z, model_vertices.back().position.z);
    }
    aabb.clamp();
    size_t total_mesh_indices = 0;
    for (auto &&shape : shapes)
    {
        total_mesh_indices += shape.mesh.indices.size();
    }
    assert(total_mesh_indices > 0);
    model_indices.reserve(total_mesh_indices);

    bool contains_normals = false, contains_uvs = false;

    for (auto &&shape : shapes)
    {
        for (auto &&index : shape.mesh.indices)
        {
            if (index.vertex_index < 0)
                throw std::runtime_error("Tinyobj Index doesn't contain vertex index. Every Index must contain it!");

            model_indices.push_back(index.vertex_index);

            if (index.normal_index >= 0)
            {
                model_vertices[index.vertex_index].normal = {attrib.normals[3 * index.normal_index + 0],
                                                             attrib.normals[3 * index.normal_index + 1],
                                                             attrib.normals[3 * index.normal_index + 2]};
                contains_normals = true;
            }
            if (index.texcoord_index >= 0)
            {
                model_vertices[index.vertex_index].uv_x = attrib.texcoords[3 * index.texcoord_index + 0];
                model_vertices[index.vertex_index].uv_y = attrib.texcoords[3 * index.texcoord_index + 1];
                contains_uvs = true;
            }
        }
    }

    if (!contains_normals)
        Logger::log(std::format("{}Mesh doesn't contain normals", filepath), LogType::Warning);
    if (!contains_uvs)
        Logger::log(std::format("{}Mesh doesn't contain normals", filepath), LogType::Info);

    return Mesh{core.allocate_gpu_mesh_buffers(model_indices, model_vertices),
                static_cast<uint32_t>(model_indices.size()), static_cast<uint32_t>(model_vertices.size()), aabb};
}
Mesh load_from_file_gltf(const Core &core, const std::string &filepath, glm::vec4 color)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;

    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);

    if (!warn.empty())
        Logger::log(warn, LogType::Warning);
    if (!err.empty())
        Logger::log(err, LogType::Error);

    if (!ret)
    {
        Logger::log(std::format("Failed to parse glTF: {}", filepath), LogType::Error);
        return {};
    }

    const tinygltf::Node &first_node = model.nodes[1];

    std::vector<uint32_t> indices{};
    std::vector<Vertex> vertices{};
    AABB aabb{};

    if (first_node.mesh > -1)
    {
        const tinygltf::Mesh mesh = model.meshes[first_node.mesh];
        const tinygltf::Primitive &primitive = mesh.primitives[0];

        uint32_t first_index = static_cast<uint32_t>(indices.size());
        uint32_t vertex_start = static_cast<uint32_t>(vertices.size());

        uint32_t index_count = 0;

        // Vertices
        {
            const float *position_buffer = nullptr;
            const float *normals_buffer = nullptr;
            const float *tex_coords_buffer = nullptr;
            size_t vertex_count = 0;

            // TODO: Else warn the user
            if (primitive.attributes.find("POSITION") != primitive.attributes.end())
            {
                const tinygltf::Accessor &accessor = model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView &buffer_view = model.bufferViews[accessor.bufferView];

                position_buffer = reinterpret_cast<const float *>(
                    &(model.buffers[buffer_view.buffer].data[accessor.byteOffset + buffer_view.byteOffset]));
                vertex_count = accessor.count;
            }
            if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
            {
                const tinygltf::Accessor &accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView &buffer_view = model.bufferViews[accessor.bufferView];
                normals_buffer = reinterpret_cast<const float *>(
                    &(model.buffers[buffer_view.buffer].data[accessor.byteOffset + buffer_view.byteOffset]));
            }
            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
            {
                const tinygltf::Accessor &accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView &buffer_view = model.bufferViews[accessor.bufferView];
                tex_coords_buffer = reinterpret_cast<const float *>(
                    &(model.buffers[buffer_view.buffer].data[accessor.byteOffset + buffer_view.byteOffset]));
            }

            for (size_t v = 0; v < vertex_count; ++v)
            {
                Vertex vertex{glm::vec3{
                                  position_buffer[v * 3 + 0],
                                  position_buffer[v * 3 + 1],
                                  position_buffer[v * 3 + 2],
                              },
                              tex_coords_buffer[v * 2 + 0],
                              glm::vec3{
                                  normals_buffer[v * 3 + 0],
                                  normals_buffer[v * 3 + 1],
                                  normals_buffer[v * 3 + 2],
                              },
                              tex_coords_buffer[v * 2 + 1], glm::vec4{1.0f}};

                aabb.min.x = std::min(aabb.min.x, vertex.position.x);
                aabb.min.y = std::min(aabb.min.y, vertex.position.y);
                aabb.min.z = std::min(aabb.min.z, vertex.position.z);

                aabb.max.x = std::max(aabb.max.x, vertex.position.x);
                aabb.max.y = std::max(aabb.max.y, vertex.position.y);
                aabb.max.z = std::max(aabb.max.z, vertex.position.z);

                vertices.push_back(vertex);
            }
        }
        aabb.clamp();

        // Indices
        {
            const tinygltf::Accessor &accessor = model.accessors[primitive.indices];
            const tinygltf::BufferView &buffer_view = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = model.buffers[buffer_view.buffer];

            index_count += static_cast<uint32_t>(accessor.count);

            switch (accessor.componentType)
            {
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                const uint32_t *buf =
                    reinterpret_cast<const uint32_t *>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
                for (size_t index = 0; index < accessor.count; ++index)
                {
                    indices.push_back(buf[index] + vertex_start);
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                const uint16_t *buf =
                    reinterpret_cast<const uint16_t *>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
                for (size_t index = 0; index < accessor.count; ++index)
                {
                    indices.push_back(buf[index] + vertex_start);
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                const uint8_t *buf =
                    reinterpret_cast<const uint8_t *>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
                for (size_t index = 0; index < accessor.count; ++index)
                {
                    indices.push_back(buf[index] + vertex_start);
                }
                break;
            }
            default:
                Logger::log(std::format("Index component type {}", accessor.componentType), LogType::Error);
                return {};
            }
        }
    }

    return {core.allocate_gpu_mesh_buffers(indices, vertices), static_cast<uint32_t>(indices.size()),
            static_cast<uint32_t>(vertices.size()), aabb};
}

Mesh Mesh::load_from_file(const Core &core, std::filesystem::path filepath, glm::vec4 color)
{
    std::string ext = filepath.extension().string();

    if (ext == ".obj")
        return load_from_file_obj(core, filepath.string(), color);
    if (ext == ".gltf" || ext == ".glb")
        return load_from_file_gltf(core, filepath.string(), color);
    Logger::log("load_from_file doesn't support extension" + ext, LogType::Error);
    return {};
}

Mesh Mesh::flat_plane(const Core &core, uint32_t length, uint32_t width, float spacing, glm::vec4 color)
{
    std::vector<Vertex> vertices;
    vertices.reserve((length + 1) * (width + 1));
    std::vector<uint32_t> indices;

    AABB aabb = {{std::max(AABB::minimum_value, int(width / 2) * spacing), AABB::minimum_value,
                  std::max(AABB::minimum_value, int(length / 2) * spacing)},
                 {std::min(-AABB::minimum_value, -int(width / 2) * spacing), -AABB::minimum_value,
                  std::min(-AABB::minimum_value, -int(length / 2) * spacing)}};

    for (int x = -int(width / 2); x <= int(width / 2); ++x)
    {
        for (int z = -int(length / 2); z <= int(length / 2); ++z)
        {
            vertices.push_back(Vertex{{x * spacing, 0.0f, z * spacing},
                                      (x + width / 2.0f) / width,
                                      {0.0f, 1.0f, 0.0f},
                                      (z + length / 2.0f) / length,
                                      color});
        }
    }
    assert(vertices.size() == (length + 1) * (width + 1));

    for (uint32_t i = 0; i < length; ++i)
    {
        for (uint32_t j = 0; j < width; ++j)
        {
            uint32_t quad_top_left = j + i * (length + 1);
            uint32_t quad_top_right = quad_top_left + 1;
            uint32_t quad_bottom_right = quad_top_right + length + 1;
            uint32_t quad_bottom_left = quad_top_left + length + 1;

            indices.push_back(quad_top_left);
            indices.push_back(quad_top_right);
            indices.push_back(quad_bottom_right);
            indices.push_back(quad_bottom_right);
            indices.push_back(quad_bottom_left);
            indices.push_back(quad_top_left);
        }
    }

    assert(indices.size() == 6 * (width * length));

    return Mesh{core.allocate_gpu_mesh_buffers(indices, vertices), static_cast<uint32_t>(indices.size()),
                static_cast<uint32_t>(vertices.size()), aabb};
}
Mesh Mesh::quad(const Core &core, glm::vec4 color)
{
    std::array<uint32_t, 6> indices{0, 1, 2, 2, 3, 0};
    std::array<Vertex, 4> vertices{
        Vertex{{-1.0f, 1.0f, 0.0f}, 0, {}, 0, color},
        Vertex{{-1.0f, -1.0f, 0.0f}, 0, {}, 0, color},
        Vertex{{1.0f, -1.0f, 0.0f}, 0, {}, 0, color},
        Vertex{{1.0f, 1.0f, 0.0f}, 0, {}, 0, color},
    };

    return Mesh{core.allocate_gpu_mesh_buffers(indices, vertices), static_cast<uint32_t>(indices.size()),
                static_cast<uint32_t>(vertices.size())};
}

Mesh Mesh::cube_interpolated_normals(const Core &core, std::array<glm::vec4, 8> vertex_colors)
{
    // clang-format off
    std::array<uint32_t, 36> indices{
        0,4,5,5,1,0,
        2,6,7,7,3,2,
        1,5,6,6,2,1,
        0,3,7,7,4,0,
        4,7,6,6,5,4,
        0,1,2,2,3,0,
    };
    // clang-format on

    std::array<Vertex, 8> vertices{
        // Bottom
        Vertex{{-1.0f, -1.0f, 1.0f}, 0.0f, {0.0f, -1.0f, 0.0f}, 0.0f, vertex_colors[0]},
        Vertex{{1.0f, -1.0f, 1.0f}, 1.0f, {0.0f, -1.0f, 0.0f}, 0.0f, vertex_colors[1]},
        Vertex{{1.0f, -1.0f, -1.0f}, 1.0f, {0.0f, -1.0f, 0.0f}, 1.0f, vertex_colors[2]},
        Vertex{{-1.0f, -1.0f, -1.0f}, 0.0f, {0.0f, -1.0f, 0.0f}, 1.0f, vertex_colors[3]},
        // Top
        Vertex{{-1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 1.0f, 0.0f}, 1.0f, vertex_colors[4]},
        Vertex{{1.0f, 1.0f, 1.0f}, 1.0f, {0.0f, 1.0f, 0.0f}, 1.0f, vertex_colors[5]},
        Vertex{{1.0f, 1.0f, -1.0f}, 1.0f, {0.0f, 1.0f, 0.0f}, 0.0f, vertex_colors[6]},
        Vertex{{-1.0f, 1.0f, -1.0f}, 0.0f, {0.0f, 1.0f, 0.0f}, 0.0f, vertex_colors[7]},
    };

    return Mesh{core.allocate_gpu_mesh_buffers(indices, vertices), static_cast<uint32_t>(indices.size()),
                static_cast<uint32_t>(vertices.size())};
}

Mesh Mesh::sphere(const Core &core, float radius, glm::vec4 color)
{
}
} // namespace ving
