# Первая стадия - сборка
FROM gcc:latest AS builder

WORKDIR /app

# Копируем исходный код
COPY main.cpp .

# Компилируем приложение
RUN g++ -o hello main.cpp -static

# Вторая стадия - выполнение
FROM alpine:latest

WORKDIR /app

# Копируем только бинарник из первой стадии
COPY --from=builder /app/hello .

# Запускаем приложение
CMD ["./hello"]

#docker build -t hello-world .
#docker run -it hello-world
# Не забыть его остановить и удалить !!!