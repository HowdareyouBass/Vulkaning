#pragma once

#include <chrono>
#include <functional>

namespace ving
{
class Profiler;
class ScopedTask final
{
    using clock = std::chrono::system_clock;

  public:
    ScopedTask(Profiler &profiler, std::string_view name);
    ~ScopedTask();

    ScopedTask(const ScopedTask &) = delete;
    ScopedTask &operator=(const ScopedTask &) = delete;
    ScopedTask(ScopedTask &&) = delete;
    ScopedTask &operator=(ScopedTask &&) = delete;

  private:
    Profiler &r_profiler;

    std::string_view m_name;

    clock::time_point m_start;
    clock::time_point m_end;
};

struct TaskInfo
{
    std::string_view name;
    std::chrono::duration<float> duration;
};

class Profiler final
{
  public:
    ScopedTask start_scoped_task(std::string_view name);

    void end_task(TaskInfo info);
    void flush();

    void imgui_frame() const;

  private:
    std::vector<TaskInfo> m_tasks;
};
} // namespace ving
