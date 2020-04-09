#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "display/my_gpio/oop_my_gpio.h"
#include "display/oop_display.h"

/* Максимальное число итераций обращений к линии */
#define _MAX_ 100000

/* Скорость звука */
#define SOUND_SPEED 340.29

typedef struct range_finder{
    Gpio *trigger;
    Gpio *echo;
    double distance;
} RangeFinder;

RangeFinder* range_finder_new(int trigger, int echo){
    RangeFinder *self = (RangeFinder*)malloc(sizeof(RangeFinder));
    self->trigger = gpio_open(trigger, "out", NULL);
    self->echo = gpio_open(echo, "in", NULL);
    self->distance = 0;
    return self;
}

void range_finder_delete(RangeFinder *self){
    gpio_close(self->trigger);
    gpio_close(self->echo);
    free(self);
}

/* Функция возвращает текущее время в микросекундах */
long long get_time(){
    /* Структура с информацией о текущем времени */
    struct timespec tspec;

    /* Запрашиваем текущее время */
    clock_gettime(CLOCK_REALTIME, &tspec);

    return tspec.tv_nsec;
}

void range_finder_calculate_distance(RangeFinder *self, long max_iter){
    long long int t = 0L;
    int k;

    /* Инициализируем датчик, притягивая TRIGGER контакт к нулю.
       Если не сделать паузу после этого, то иногда происходит ошибка
       работы датчика. */
    gpio_write(self->trigger, 0);
    usleep(500000);

    /* Инициируем излучение 8 пачек ультразвуковых сигналов, притягивая
       контакт TRIGGER к 1 на 10 мкс */
    gpio_write(self->trigger, 1);
    usleep(10);
    gpio_write(self->trigger, 0);

    /* Ожидаем, когда входном сигнале ECHO будет логический ноль. */
    for(k = 0; k < max_iter && gpio_read(self->echo) == 0; k++);
    t = get_time();

    /* Ждем эхо-сигнал */
    for(k = 0; k < max_iter && gpio_read(self->echo) == 1; k++);
    t = get_time() - t;

    self->distance = ((1.0 * t / 1000000000.0) * SOUND_SPEED) / 2.0;
}

int main() {
    char buffer[6];
    Gpio *button = gpio_open(6, "in", "rising");
    RangeFinder *device = range_finder_new(12, 11);
    Display *led = display_new(9, 10, 20);
    bool true_points[] = {false, true, false, false};
    unsigned char true_values[4] = {buffer[0] - 48, buffer[1] - 48, buffer[3] - 48, buffer[4] - 48};
    bool points[] = {false, false, false, false};
    unsigned char values[4] = {11, 11, 11, 11};
    int counter = 0;
    while (true){
        if (gpio_poll(button, 1) == 1){
            printf("%d\n", counter);
            range_finder_calculate_distance(device, 100000);
            sprintf(buffer, "%05.2f", device->distance);
            puts(buffer);
            printf("%f\n", device->distance);
            if (device->distance >= 0.02 && device->distance <= 4.0){
                unsigned char true_values[4] = {buffer[0] - 48, buffer[1] - 48, buffer[3] - 48, buffer[4] - 48};
                display_print_values(led, true_values, true_points);
            } else{
                display_print_values(led, values, points);
            }
            if (++counter == 5){
                break;
            }
        }
    }
    getchar();
    display_clear(led);
    display_delete(led);
    range_finder_delete(device);
    return 0;
}
