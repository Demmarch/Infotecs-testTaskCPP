#pragma once

#include <string>

// Сборник всякой всячины
namespace utils {
[[noreturn]] void throw_system_error(const std::string& context_msg);
}
