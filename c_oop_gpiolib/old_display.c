#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <my_gpio.h>

/* SCLK соединен с 8 линией GPIO */
#define SCLK   8

/* RCLK соединен с 10 линией GPIO */
#define RCLK 10

/* DIO соединен с 11 линией GPIO */
#define DIO   11

/* Константа для включения разделительных точек 
   на LED индикаторе */
#define POINT_ON        0x7F

/* Буфер для отображаемы на индикаторе символов */
unsigned char buf[4];

/* Функция для отображения символа на индикаторе */
void send(unsigned char X){
    unsigned char i = 0;

    for(i = 8; i >= 1; i--){
        if(X & 0x80){
            gpio_write(DIO, 1);
        }else{
             gpio_write(DIO, 0);
        }
        X <<= 1;

        gpio_write(SCLK, 0);
        gpio_write(SCLK, 1);
    }
}

/* Цикл обновления знаков для динамической индикации */
void *display_update(){
    while(1){
        int didgit = 0;
        for(didgit = 0; didgit < 4; didgit++){
                int position =  1 << didgit;
                gpio_write(RCLK, 0);
                send(buf[didgit]);
                send(position);
                gpio_write(RCLK, 1);
                usleep(5000);
        }
    }
}

/* Запуск нити с циклом для перебора знаков для динамической 
   индикации */
void display_update_init(){
    pthread_t thread;
    int status = pthread_create(&thread, NULL, display_update, NULL);
    if (status != 0) {
        printf("main error: can't create thread, status = %d\n", status);
        exit(1);
    }
}

/* Display value X at position A position */
void display_value(unsigned char A, unsigned char X){
    buf[A] = X;
}

/* Display W, X, Y and Z values starting from position 0 */
void display_values(unsigned char W, unsigned char X, unsigned char Y, unsigned char Z){
    display_value(0, W);
    display_value(1, X);
    display_value(2, Y);
    display_value(3, Z);
}

/* Clear LED display */
void clear_display(){
    display_values(0xFF, 0xFF, 0xFF, 0xFF);
}

int main(int argc, char *argv[]){

    /* Открываем GPIO линии */
    gpio_open(SCLK, "out", "none");
    gpio_open(RCLK, "out", "none");
    gpio_open(DIO, "out", "none");

    /* Запускаем нить динамической индикации */
    display_update_init();

    /* Очищаем индикатор */
    clear_display();

    /* Выводим на индиктор 32.10 */
    display_values(0xC0, 0xF9, 0xA4 & POINT_ON, 0xB0);

    while(1){
        usleep(10000);
    }

    /* Мы сюда никогда не попадем */ 
    // close(SCLK);
    // close(RCLK);
    // close(DIO);

    return 0;
}

