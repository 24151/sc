# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Исходники и бинарник
SRC = sat.c
TARGET = main

# Сборка
all: $(TARGET)

$(TARGET): $(SRC)
	@ $(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Интерактивный запуск
run: $(TARGET)
	@ read -p "Введите путь .sa файла: " SA; \
	read -p "Введите путь итогового файла: " MEM; \
	./$(TARGET) $$SA $$MEM

# Очистка
clean:
	@rm -f $(TARGET)

.PHONY: all run clean
