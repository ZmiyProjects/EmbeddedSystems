#include <stdbool.h>
#include <pthread.h>

#include "oop_my_gpio.h"


#ifndef OOP_DISPLAY_C_OOP_DISPLAY_H
#define OOP_DISPLAY_C_OOP_DISPLAY_H

typedef struct display{
    int permissible_values[12];
    bool show_status;
    pthread_t update_thread;
    unsigned char numbers[4];
    Gpio* gpio_sclk;
    Gpio* gpio_rclk;
    Gpio* gpio_dio;
} Display;

// Конструктор объекта display
Display *display_new(int sclk, int rclk, int dio);

// Деструктор объекта Display (очистка памяти и закрытие GPIO)
void display_delete(Display *self);

// Цикл обновления знаков для динамической индикации
void *display_update(void *args);

//
//void display_init(Display *self, void (*update_func)(void *arg), void *args);

void display_print_value(Display *self, unsigned char index, unsigned char value);

void display_print_values(Display *self, unsigned char values[]);

void display_clear(Display *self);

/* Функция для отображения символа на индикаторе */
void display_send(Display *self, unsigned char value);

#endif //OOP_DISPLAY_C_OOP_DISPLAY_H
