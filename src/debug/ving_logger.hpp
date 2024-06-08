#pragma once

#include <string_view>

namespace ving
{
enum LogType
{
    Info,
    Warning
};
class Logger
{
  public:
    static void Log(std::string_view string, LogType type);
};
} // namespace ving
