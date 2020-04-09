#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#include "my_gpio/oop_my_gpio.h"
#include "oop_display.h"

/* Константа для включения разделительных точек 
   на LED индикаторе */
#define POINT_ON        0x7F

Display *display_new(int sclk, int rclk, int dio){
    // перечень допустимых значений, отображемых на индикаторе, 10 (0xFF) - ничего, 11 (0xBF) - прочерк
    int nums[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF, 0xBF};
    Display *self = (Display*)malloc(sizeof(Display));
    self->gpio_sclk = gpio_open(sclk, "out", "none");
    self->gpio_rclk = gpio_open(rclk, "out", "none");
    self->gpio_dio = gpio_open(dio, "out", "none");
    self->show_status = true;
    for(int i=0; i<12; i++){
        self->permissible_values[i] = nums[i];
    }
    pthread_create(&self->update_thread, NULL, display_update, self);
    display_clear(self);
    return self;
}


// Дуструктор для Display
void display_delete(Display *self){
    gpio_close(self->gpio_dio);
    gpio_close(self->gpio_rclk);
    gpio_close(self->gpio_sclk);
    free(self);
}

/* Функция для отображения символа на индикаторе */
void display_send(Display *self, unsigned char value){
    unsigned char i = 0;
    for(i = 8; i >= 1; i--){
        if(value & 0x80){
            gpio_write(self->gpio_dio, 1);
        }else{
             gpio_write(self->gpio_dio, 0);
        }
        value <<= 1;
        gpio_write(self->gpio_sclk, 0);
        gpio_write(self->gpio_sclk, 1);
    }
}

/* Цикл обновления знаков для динамической индикации */
void *display_update(void *args){
    Display *self = (Display*)args;
    while(self->show_status){
        for(int digit = 0; digit < 4; digit++){
                int position =  1 << digit;
                gpio_write(self->gpio_rclk, 0);
                display_send(self, self->numbers[digit]);
                display_send(self, position);
                gpio_write(self->gpio_rclk, 1);
                usleep(5000);
        }
    }
}

void display_print_value(Display *self, unsigned char index, unsigned char value, bool point){
    if (point) {
        self->numbers[3 - index] = self->permissible_values[value] & POINT_ON;
    } else{
        self->numbers[3 - index] = self->permissible_values[value];
    }
}

void display_print_values(Display *self, unsigned char values[], bool points[]){
    for(int i=0; i<4; i++){
        display_print_value(self, i, values[i], points[i]);
    }
}


/* Clear LED display */
void display_clear(Display *self){
    static unsigned char values[] = {10, 10, 10, 10};
    static bool points[] = {false, false, false, false};
    display_print_values(self, values, points);
    usleep(5001);
}
