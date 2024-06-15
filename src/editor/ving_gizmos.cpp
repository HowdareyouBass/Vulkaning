#include "ving_gizmos.hpp"

namespace ving
{
namespace editor
{
std::array<AABB, 3> Gizmo::make_gizmo_aabbs(float length)
{
    return {
        // X
        AABB{{0.0f, -gizmo_aabb_offset, -gizmo_aabb_offset}, {length, gizmo_aabb_offset, gizmo_aabb_offset}},
        // Y
        AABB{{-gizmo_aabb_offset, 0.0f, -gizmo_aabb_offset}, {gizmo_aabb_offset, length, gizmo_aabb_offset}},
        // Z
        AABB{{-gizmo_aabb_offset, -gizmo_aabb_offset, 0.0f}, {gizmo_aabb_offset, gizmo_aabb_offset, length}},
    };
}
} // namespace editor
} // namespace ving
