#include <time.h>
#include <unistd.h>

#include "oop_my_gpio.h"
#include "oop_display.h"

#ifndef OOP_HC_SR04_C_OOP_HC_SR04_H
#define OOP_HC_SR04_C_OOP_HC_SR04_H

typedef struct range_finder{
    double distance;
    Gpio* echo;
    Gpio* trigger;
} RangeFinder;

/* Функция возвращает текущее время в микросекундах */
long long get_time(void);

RangeFinder* range_finder_new(int echo, int trigger);

void range_finder_delete(RangeFinder* self);

float range_finder_calculate_distance(RangeFinder* self, unsigned long max_iter);

#endif //OOP_HC_SR04_C_OOP_HC_SR04_H
