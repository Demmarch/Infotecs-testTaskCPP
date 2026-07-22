#include "utils.hpp"
#include <cerrno>
#include <system_error>

namespace utils {

void throw_system_error(const std::string& context_msg) {
    int error_number = errno;
    std::error_code ec(error_number, std::system_category());
    throw std::system_error(ec, context_msg);
}

} // namespace utils
