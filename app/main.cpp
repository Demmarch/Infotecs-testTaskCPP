#include "../lib/logger.hpp"
#include "thread_safe_queue.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

struct LogTask {
    LogLevel level;
    std::string message;
    bool is_exit = false;
};

// Возвращаем optional, чтобы легко проверять, распознан уровень или нет
std::optional<LogLevel> try_parse_level(std::string_view level_str) {
    if (level_str == "DEBUG") return LogLevel::DEBUG;
    if (level_str == "INFO") return LogLevel::INFO;
    if (level_str == "WARNING") return LogLevel::WARNING;
    if (level_str == "ERROR") return LogLevel::ERROR;
    return std::nullopt;
}

LogLevel parse_level(std::string_view level_str, LogLevel default_level) {
    return try_parse_level(level_str).value_or(default_level);
}

LogTask parse_input(std::string_view input, LogLevel default_level) {
    LogTask task;
    task.level = default_level;

    size_t colon_pos = input.find(':');
    if (colon_pos != std::string_view::npos) {
        std::string_view prefix = input.substr(0, colon_pos);

        if (auto parsed = try_parse_level(prefix); parsed.has_value()) {
            task.level = parsed.value();
        } else {
            colon_pos = std::string_view::npos;
        }
    }

    std::string_view msg_view =
        (colon_pos != std::string_view::npos) ? input.substr(colon_pos + 1) : input;

    size_t first_char = msg_view.find_first_not_of(" \t");
    if (first_char != std::string_view::npos) {
        task.message = std::string(msg_view.substr(first_char));
    }

    return task;
}

class ThreadJoiner {
    std::thread& t_;

public:
    explicit ThreadJoiner(std::thread& t) : t_(t) {}
    ~ThreadJoiner() {
        if (t_.joinable()) { t_.join(); }
    }
};

void logger_worker(Logger& logger, ThreadSafeQueue<LogTask>& queue) {
    try {
        while (true) {
            LogTask task;
            queue.wait_and_pop(task);

            if (task.is_exit) { break; }

            logger.log(task.level, task.message);
        }
    } catch (const std::exception& e) {
        std::cerr << "[Фоновый поток] Ошибка записи: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    // ПО ТЗ Параметрами приложения должны быть имя файла журнала и уровень по умолчанию
    if (argc < 3) {
        std::cerr << "Использование: " << argv[0]
                  << " <файл_журнала> <уровень_по_умолчанию (DEBUG|INFO|WARNING|ERROR)>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::string default_level_str = argv[2];
    LogLevel default_level = parse_level(default_level_str, LogLevel::INFO);

    try {
        // Создаем логгер и очередь
        Logger logger(filename, default_level);
        ThreadSafeQueue<LogTask> queue;

        // Запускаем фоновый поток, передавая объекты по ссылке
        std::thread worker(logger_worker, std::ref(logger), std::ref(queue));
        ThreadJoiner joiner(worker);

        std::cout << "Приложение запущено. Вводите сообщения для записи.\n";
        std::cout << "Формат ввода: [УРОВЕНЬ:] текст сообщения\n";
        std::cout << "Например: 'ERROR: нет подключения' или просто 'тестовое сообщение'\n";
        std::cout << "Для выхода введите 'exit' или нажмите Ctrl+D.\n";

        std::string input;
        while (true) {
            std::cout << "> ";

            // Читаем ввод. std::getline вернет false при EOF (Ctrl+D)
            if (!std::getline(std::cin, input) || input == "exit") { break; }

            if (input.empty()) { continue; }

            // Парсим ввод и отправляем задачу в очередь
            LogTask task = parse_input(input, default_level);
            queue.push(task);
        }

        // Штатное завершение работы (Poison Pill)
        LogTask exit_task;
        exit_task.is_exit = true;
        queue.push(exit_task);

        // Обязательно ждем завершения фонового потока, иначе программа упадет (std::terminate)
        worker.join();

        std::cout << "Завершение работы.\n";

    } catch (const std::exception& e) {
        // Ловим ошибки инициализации (например, если файл недоступен)
        std::cerr << "Критическая ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
