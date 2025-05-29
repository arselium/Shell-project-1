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

# Установка переменной окружения HOME
ENV HOME=/home/user
# Создаём домашний каталог
RUN mkdir -p /home/user

# Копируем только бинарник из первой стадии
COPY --from=builder /app/hello .

# Запускаем приложение
CMD ["./hello"]

#docker build -t hello-world .
#docker run -it --rm hello-world