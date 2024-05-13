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
        X,
        Y,
        Z
    };
    static constexpr float gizmo_aabb_offset = 0.05f;

    static std::array<AABB, 3> make_gizmo_aabbs(const SceneObject &scene_object);

    constexpr static float length = 0.5f;
};
} // namespace editor
} // namespace ving
