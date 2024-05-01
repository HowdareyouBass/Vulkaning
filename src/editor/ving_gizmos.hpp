#pragma once

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

    constexpr static float length = 0.5f;
};
} // namespace editor
} // namespace ving
