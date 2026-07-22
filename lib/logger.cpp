#include "logger.hpp"
#include "utils.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

Logger::Logger(const std::string& filename, LogLevel default_level)
    : current_level_(default_level) {
    // Открыть файл
    // Режим: добавление (самоограничение, чтобы не удалить старые логи)
    file_stream_.open(filename, std::ios::app);

    // Бросаем ошибку если не открылся файлик
    if (!file_stream_.is_open()) {
        utils::throw_system_error("Не удалось открыть файл журнала: " + filename);
    }
}

Logger::~Logger() {
    if (file_stream_.is_open()) { file_stream_.close(); }
}

void Logger::set_level(LogLevel new_level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = new_level;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    // По ТЗ - сообщения с уровнем ниже заданного не должны записываться
    if (level < current_level_) { return; }

    // Текст, уровень, время
    std::string log_entry =
        "[" + get_current_time() + "] " + "[" + level_to_string(level) + "] " + message + "\n";

    file_stream_ << log_entry;
    file_stream_.flush(); // Форсируем запись на диск, чтобы логи не терялись при краше

    // Обработка ошибок
    if (file_stream_.fail()) {
        utils::throw_system_error("Критическая ошибка при записи в файл журнала");
    }
}

std::string Logger::level_to_string(LogLevel level) const {
    switch (level) {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

std::string Logger::get_current_time() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
