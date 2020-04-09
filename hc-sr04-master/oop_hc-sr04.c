#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "oop_hc-sr04.h"
#include "oop_my_gpio.h"
#include "oop_display.h"

/* Скорость звука */
#define SOUND_SPEED 340.29

/* Функция возвращает текущее время в микросекундах */
long long get_time(){
    /* Структура с информацией о текущем времени */
    struct timespec tspec;
    /* Запрашиваем текущее время */
    clock_gettime(CLOCK_REALTIME, &tspec);
    return tspec.tv_nsec;
}

RangeFinder* range_finder_new(int echo, int trigger){
    RangeFinder *self = (RangeFinder*)malloc(sizeof(RangeFinder));
    self->echo = gpio_open(echo, "out", NULL);
    self->trigger = gpio_open(trigger, "in", NULL);
    self->distance = 0;
    printf("%d\n", self->echo->value_fd);
    printf("%d\n", self->trigger->value_fd);
    printf("%c\n", gpio_read(self->echo));
    printf("%c\n", gpio_read(self->trigger));
    return self;
}

void range_finder_delete(RangeFinder *self){
    gpio_close(self->echo);
    gpio_close(self->trigger);
    free(self);
}

float range_finder_calculate_distance(RangeFinder *self, unsigned long max_iter){
    Gpio *Trigger = gpio_open(self->trigger->sysfs_id, "out", NULL);
    Gpio *Echo = gpio_open(self->trigger->sysfs_id, "out", NULL);
    printf("%lu\n", max_iter);
    long long int t = 0L;
    int k;
    /* Инициализируем датчик, притягивая TRIGGER контакт к нулю.
       Если не сделать паузу после этого, то иногда происходит ошибка
       работы датчика. */
    gpio_write(Trigger, 0);
    usleep(500000);
    /* Инициируем излучение 8 пачек ультразвуковых сигналов, притягивая
       контакт TRIGGER к 1 на 10 мкс */
    gpio_write(Trigger, 1);
    usleep(10);
    gpio_write(Trigger, 0);
    /* Ожидаем, когда входном сигнале ECHO будет логический ноль. */
    for(k = 0; k < max_iter && gpio_read(Echo) == 0; k++);
    t = get_time();
    /* Ждем эхо-сигнал */
    for(k = 0; k < max_iter && gpio_read(Echo) == 1; k++);
    t = get_time() - t;
    float distance = ((1.0 * t / 1000000000.0) * SOUND_SPEED) / 2.0;
    self->distance = distance;
    return self->distance;
}
