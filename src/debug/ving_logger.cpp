#include "ving_logger.hpp"
#include <iostream>
#include <string>

namespace ving
{
void Logger::Log(std::string_view string, LogType type)
{
    std::string prefix;
    switch (type)
    {
    case LogType::Warning: {
        prefix = "WARN: ";
        break;
    }
    case LogType::Info: {
        prefix = "INFO: ";
        break;
    }
    }

    std::cout << prefix << string << '\n';
}
} // namespace ving
