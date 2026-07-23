#pragma once

#include <memory>
#include <string>

// Уровень через перечисление с понятными именами
// enum class чтобы не превращались в числа
enum class LogLevel { DEBUG = 0, INFO, WARNING, ERROR };

class Logger {
public:
    // Инициализация логгера
    Logger(const std::string& filename, LogLevel default_level);

    // Деструктор закроет файл автоматически
    ~Logger();

    // Запрещаем копирование и присваивание объекта логгера
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Требование: возможность менять уровень важности после инициализации
    void set_level(LogLevel new_level);

    // Главный метод записи в журнал
    void log(LogLevel level, const std::string& message);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
