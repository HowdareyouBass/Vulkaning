#include "ving_scene_object.hpp"
#include "ving_color.hpp"
#include "ving_core.hpp"

#include <iostream>
#include <tiny_obj_loader.h>

namespace ving
{
Mesh Mesh::load_from_file(const Core &core, std::string_view filepath, glm::vec4 color)
{
    tinyobj::attrib_t attrib;
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
    }

    size_t total_mesh_indices = 0;
    for (auto &&shape : shapes)
    {
        total_mesh_indices += shape.mesh.indices.size();
    }
    assert(total_mesh_indices > 0);
    model_indices.reserve(total_mesh_indices);

    for (auto &&shape : shapes)
    {
        for (auto &&index : shape.mesh.indices)
        {
            if (index.vertex_index < 0)
                throw std::runtime_error("Tinyobj Index doesn't contain vertex index. Every Index must contain it!");

            model_indices.push_back(index.vertex_index);

            // TODO: Log Warning if model doesn't conatin normals or texcoords
            if (index.normal_index >= 0)
            {
                model_vertices[index.vertex_index].normal = {attrib.normals[3 * index.normal_index + 0],
                                                             attrib.normals[3 * index.normal_index + 1],
                                                             attrib.normals[3 * index.normal_index + 2]};
            }
            if (index.texcoord_index >= 0)
            {
                model_vertices[index.vertex_index].uv_x = attrib.texcoords[3 * index.texcoord_index + 0];
                model_vertices[index.vertex_index].uv_y = attrib.texcoords[3 * index.texcoord_index + 1];
            }
        }
    }

    return Mesh{core.allocate_gpu_mesh_buffers(model_indices, model_vertices),
                static_cast<uint32_t>(model_indices.size()), static_cast<uint32_t>(model_vertices.size())};
}

// TODO: Generate uv and normals
Mesh SimpleMesh::flat_plane(const Core &core, uint32_t length, uint32_t width, float spacing, glm::vec4 color)
{
    std::vector<Vertex> vertices;
    vertices.reserve((length + 1) * (width + 1));
    std::vector<uint32_t> indices;

    for (int x = -int(width / 2); x <= int(width / 2); ++x)
    {
        for (int z = -int(length / 2); z <= int(length / 2); ++z)
        {
            vertices.push_back(Vertex{{x * spacing, 0.0f, z * spacing}, 0, {0.0f, 1.0f, 0.0f}, 0, color});
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
                static_cast<uint32_t>(vertices.size())};
}
Mesh SimpleMesh::quad(const Core &core, glm::vec4 color)
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

Mesh SimpleMesh::cube_interpolated_normals(const Core &core, std::array<glm::vec4, 8> vertex_colors)
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

RayTracedMesh SimpleMesh::cube_raytraced(const Core &core, vk::TransformMatrixKHR &transform,
                                         std::array<glm::vec4, 8> vertex_colors)
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

    GPUBuffer vertex_buffer =
        core.create_gpu_buffer(vertices.data(), sizeof(Vertex) * vertices.size(),
                               vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                   vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
    GPUBuffer index_buffer =
        core.create_gpu_buffer(indices.data(), sizeof(uint32_t) * indices.size(),
                               vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                   vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
    GPUBuffer transform_buffer =
        core.create_gpu_buffer(&transform, sizeof(transform),
                               vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                   vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

    return {std::move(vertex_buffer), std::move(index_buffer), std::move(transform_buffer), vertices.size()};
}
glm::mat4 Transform::mat4() const noexcept
{
    glm::mat4 mtx{1.0f};

    // mtx[1][1] = -mtx[1][1];

    mtx = glm::translate(mtx, translation);
    mtx = glm::scale(mtx, scale);
    mtx = glm::rotate(mtx, rotation.y, {0.0f, 1.0f, 0.0f});
    mtx = glm::rotate(mtx, rotation.x, {1.0f, 0.0f, 0.0f});
    mtx = glm::rotate(mtx, rotation.z, {0.0f, 0.0f, 1.0f});

    return mtx;
}
} // namespace ving
