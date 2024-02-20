#include <imgui.h>

#include "ving_profilers.hpp"

namespace ving
{
ScopedTask::ScopedTask(Profiler &profiler, std::string_view name) : r_profiler{profiler}, m_name{name}
{
    m_start = clock::now();
}
ScopedTask::~ScopedTask()
{
    m_end = clock::now();

    r_profiler.end_task(TaskInfo{m_name, m_end - m_start});
}

ScopedTask Profiler::start_scoped_task(std::string_view name)
{
    return ScopedTask{*this, name};
}
void Profiler::end_task(TaskInfo info)
{
    m_tasks.push_back(info);
}
void Profiler::flush()
{
    m_tasks.clear();
}
void Profiler::imgui_frame() const
{
    for (auto &&task : m_tasks)
    {
        ImGui::Text("%s: %.2f ms", task.name.data(), task.duration.count() * 1000.0f);
    }
}
} // namespace ving
