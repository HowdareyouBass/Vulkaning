#pragma once

#include <string_view>

namespace ving
{
enum LogType
{
    Info,
    Warning,
    Error
};
class Logger
{
  public:
    static void log(std::string_view string, LogType type);
};
} // namespace ving
