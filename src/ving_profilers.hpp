#pragma once

#include <chrono>
#include <functional>

namespace ving
{
class Profiler;

struct TaskInfo final
{
    std::string_view name;
    std::chrono::duration<float> duration;
};

class Task final
{
    using clock = std::chrono::system_clock;

  public:
    Task(Profiler &profiler, std::string_view name);

    ~Task() = default;
    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;
    Task(Task &&) = delete;
    Task &operator=(Task &&) = delete;

    void stop();

  private:
    Profiler &r_profiler;

    std::string_view m_name;

    clock::time_point m_start;
    clock::time_point m_end;
};

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
    Task m_task;
};

class Profiler final
{
  public:
    [[nodiscard]] ScopedTask start_scoped_task(std::string_view name);

    void end_task(TaskInfo info);
    void flush();

    void imgui_frame() const;

  private:
    std::vector<TaskInfo> m_tasks;
};
} // namespace ving
