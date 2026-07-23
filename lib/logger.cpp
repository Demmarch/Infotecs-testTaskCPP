#include "logger.hpp"
#include "utils.hpp"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

struct Logger::Impl {
    std::ofstream file_stream_;
    LogLevel current_level_;
    std::mutex mutex_;

    Impl(const std::string& filename, LogLevel default_level) : current_level_(default_level) {
        // Режим: добавление (самоограничение, чтобы не удалить старые логи)
        file_stream_.open(filename, std::ios::app);

        if (!file_stream_.is_open()) {
            utils::throw_system_error("Не удалось открыть файл журнала: " + filename);
        }
    }

    ~Impl() {
        if (file_stream_.is_open()) file_stream_.close();
    }

    std::string level_to_string(LogLevel level) const {
        switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
        }
    }

    std::string get_current_time() const {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);

        // localtime_r лучше и потокобезопаснее предыдущего варианта
        std::tm tm_buf{};
        localtime_r(&time_t_now, &tm_buf);

        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

Logger::Logger(const std::string& filename, LogLevel default_level)
    : impl_(std::make_unique<Impl>(filename, default_level)) {}

Logger::~Logger() = default;

void Logger::set_level(LogLevel new_level) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    impl_->current_level_ = new_level;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);

    // По ТЗ - сообщения с уровнем ниже заданного не должны записываться
    if (level < impl_->current_level_) { return; }

    // Текст, уровень, время
    std::string log_entry =
        "[" + impl_->get_current_time() + "] " + "[" + impl_->level_to_string(level) + "] " + message + "\n";

    impl_->file_stream_ << log_entry;
    impl_->file_stream_.flush(); // Форсируем запись на диск, чтобы логи не терялись при краше

    // Обработка ошибок
    if (impl_->file_stream_.fail()) {
        utils::throw_system_error("Критическая ошибка при записи в файл журнала");
    }
}
