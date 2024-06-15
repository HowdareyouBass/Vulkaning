#pragma once

#include "ving_scene_object.hpp"

namespace ving
{
namespace editor
{
struct Gizmo
{
    enum Type
    {
        Pos,
        Rotate,
        Scale,
    };
    enum Coordinate
    {
        X = 0,
        Y = 1,
        Z = 2
    };
    static constexpr float gizmo_aabb_offset = 0.08f;

    static std::array<AABB, 3> make_gizmo_aabbs(float length);
};
} // namespace editor
} // namespace ving
