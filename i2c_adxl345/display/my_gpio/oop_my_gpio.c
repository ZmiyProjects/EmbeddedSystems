#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>

#include "oop_my_gpio.h"

// Структура для работы с gpio
typedef struct Gpio {
    int value_fd;
    int direction_fd;
    int edge_fd;
    int sysfs_id;
} Gpio;

// "Конструктор" для структуры gpio
Gpio* gpio_open(int line, const char *direction, const char *edge){
    Gpio* self = (Gpio*)malloc(sizeof(Gpio));
    self->sysfs_id = line;
    char filename[PATH_MAX];
    int fd = 0;
    int len = 0;
    char buf[255];

    /* Экспортируем GPIO линию */
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if(fd == 0){
        perror("Open export file failed!");
        exit(1);
    }
    len = sprintf(buf, "%d", self->sysfs_id);
    write(fd, buf, len);
    close(fd);

    // direction
    snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/direction", self->sysfs_id);
    self->direction_fd = open(filename, O_WRONLY|O_SYNC);
    if(self->direction_fd < 0){
        perror("gpio_open() failed!");
        exit(1);
    }

    // edge
    snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/edge", self->sysfs_id);
    self->edge_fd = open(filename, O_WRONLY);
    if(self->edge_fd < 0){
        perror("gpio_open() failed!");
        exit(1);
    }

    /* Открываем файл value, соответствующий линии GPIO */
    snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/value", self->sysfs_id);
    self->value_fd = open(filename, O_RDWR|O_SYNC);
    if (self->value_fd < 0){
        perror("gpio_open() failed!");
        exit(1);
    }

    if(direction != NULL){
        gpio_set_direction(self, direction);
    }

    if(edge != NULL){
        gpio_set_edge(self, edge);
    }
	
	return self;
}

/* Установка режима работы линии GPIO */
void gpio_set_direction(Gpio *self, const char *direction){
    /* Запись должна осуществляться строго в нулевую позицию файла */
    lseek(self->direction_fd, 0, SEEK_SET);
    write(self->direction_fd, direction, strlen(direction));
}

/* Запись (установка) состояния линии GPIO */
void gpio_write(Gpio *self, char value){
    char c = value + '0';
    lseek(self->value_fd, 0, SEEK_SET);
    write(self->value_fd, &c, sizeof(c));
}

void gpio_set_edge(Gpio* self, const char* edge){
    lseek(self->edge_fd, 0, SEEK_SET);
    write(self->edge_fd, edge, strlen(edge));
}

/* Чтение (считывание) состояния линии в обычном режиме */
char gpio_read(Gpio *self){
    char value = 0;
    lseek(self->value_fd, 0, SEEK_SET);
    read(self->value_fd, &value, sizeof(value));

    /* Переводим ascii код символов '0' и '1' в соответствующие 
       целочисленные значения 0 и 1 */
    return value - '0';
}

/* Чтение (считывание) состояния линии в polling режиме */
char gpio_poll(Gpio *self, int timeout){
    struct pollfd pollfd[1];
    int err;

    /* Настраиваем polling режим считывания */
    pollfd[0].fd = self->value_fd;
    pollfd[0].events = POLLPRI | POLLERR;
    pollfd[0].revents = 0;

    err =  poll(pollfd, 1, timeout);
    if(err <= 0){
        /* Если ошибка или timeout, то возвращаем -1 */
        return -1;
    }

    return gpio_read(self);
}

/* Функция завершающая работу с GPIO линией */
void gpio_close(Gpio *self){
    int fd = 0;
    int len = 0;
    char buf[255];

    /* Закрываем файл файл value, соответствующий линии GPIO */
    if(self->value_fd > 0){
        close(self->value_fd);
    }

    /* Закрываем файл файл direction, соответствующий линии GPIO */
    if(self->direction_fd > 0){
        close(self->direction_fd);
    }

    /* Отменяем экспорт GPIO линии */
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(fd == 0){
         perror("gpio_unexport failed!");
         exit(1);
    }

    len = sprintf(buf, "%d\n", self->sysfs_id);
    write(fd, buf,len);
    close(fd);
    free(self);
}

