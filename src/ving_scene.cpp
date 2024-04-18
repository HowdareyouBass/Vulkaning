#include "ving_scene.hpp"
#include "ving_utils.hpp"

namespace ving
{
Scene::Scene(const Core &core, std::string_view skybox_filepath)
{
    skybox_cubemap = ving::utils::load_cube_map(skybox_filepath, core);
}
} // namespace ving
