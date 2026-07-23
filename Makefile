# Компилятор
CXX = g++
# Флаги компиляции
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -fPIC -I./lib
# Флаги линковки
# -Wl,-rpath,. указывает бинарнику искать .so файл (библиотеку) в текущей директории
LDFLAGS = -L. -llogger -Wl,-rpath,. -pthread

# Имена итоговых файлов
LIB_OUT = liblogger.so
APP_OUT = logger_app

# Исходные файлы и объектные файлы (ищутся автоматически)
LIB_SRC = $(wildcard lib/*.cpp)
LIB_OBJ = $(LIB_SRC:.cpp=.o)

APP_SRC = $(wildcard app/*.cpp)
APP_OBJ = $(APP_SRC:.cpp=.o)

# Указываем, что эти цели не являются файлами
.PHONY: all lib app clean rebuild

# Цель по умолчанию (собирает всё)
all: lib app

# Раздельные цели сборки, как требует задание
lib: $(LIB_OUT)

app: $(APP_OUT)

# Правило сборки динамической библиотеки
$(LIB_OUT): $(LIB_OBJ)
	$(CXX) -shared -o $@ $^

# Правило сборки исполняемого приложения
$(APP_OUT): $(APP_OBJ) $(LIB_OUT)
	$(CXX) $(APP_OBJ) -o $@ $(LDFLAGS)

# Универсальное правило для компиляции .cpp в .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Очистка от временных файлов и бинарников
clean:
	rm -f lib/*.o app/*.o $(LIB_OUT) $(APP_OUT)

rebuild: clean all
