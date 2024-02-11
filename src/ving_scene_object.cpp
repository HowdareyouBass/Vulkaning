#include "ving_scene_object.hpp"
#include "ving_core.hpp"

namespace ving
{
Mesh SimpleMesh::flat_plane(const Core &core, uint32_t length, uint32_t width)
{
    Mesh plane;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (int x = -int(width / 2); x <= int(width / 2); ++x)
    {
        for (int z = -int(length / 2); z <= int(length / 2); ++z)
        {
            vertices.push_back(Vertex{{x * 0.05f, 0.0f, z * 0.05f}, 0, {}, 0, {1.0f, 1.0f, 1.0f, 1.0f}});
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
} // namespace ving
