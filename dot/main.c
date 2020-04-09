#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "my_spi.h"
#include "my_gpio/oop_my_gpio.h"
#include <pthread.h>

/* Имя специального файла SPI шины */
#define SPI_DEV_FILE "/dev/spidev1.0"

typedef struct dot_matrix{
    int matrix_fd;

} DotMatrix;

typedef struct pointController{
    int status;
    DotMatrix* Matrix;
    Gpio* UpButton;
    Gpio* RightButton;
    Gpio* DownButton;
    Gpio* LeftButton;
    int x;
    int y;
} PointController;

void dot_matrix_set_command(DotMatrix* self, unsigned char address, unsigned char data);
DotMatrix* dot_matrix_create(char* spi_dev_file, int speed);
void dot_matrix_clean(DotMatrix* self);
void dot_matrix_delete(DotMatrix* self);
void dot_matrix_set(DotMatrix* self, int y, int x);

void* point_controller_move_up(void* args);
void* point_controller_move_down(void* args);
void* point_controller_move_left(void* args);
void* point_controller_move_right(void* args);

PointController* point_controller_new(
        char* spi_dev_file,
        int speed, int red_id,
        int blue_id,
        int white_id,
        int black_id);

void point_controller_delete(PointController* self);



int main() {
    // red = up
    // white = down
    // blue = right
    // black = left
    PointController* controller = point_controller_new(
            SPI_DEV_FILE, 10000000, 11, 12, 0, 1);

    getchar();
    controller->status = 0;
    dot_matrix_clean(controller->Matrix);
    point_controller_delete(controller);
    return 0;
}

void dot_matrix_set_command(DotMatrix* self, unsigned char address, unsigned char data){
    unsigned char buf[] = {address, data};
    spi_write(self->matrix_fd, buf, 2);
}

DotMatrix* dot_matrix_create(char* spi_dev_file, int speed){
    DotMatrix* self = (DotMatrix*)malloc(sizeof(DotMatrix));
    self->matrix_fd = spi_open(spi_dev_file, speed);
    dot_matrix_set_command(self, 0x9, 0);
    dot_matrix_set_command(self, 0xA, 3);
    dot_matrix_set_command(self, 0xB, 7);
    dot_matrix_set_command(self, 0xC, 1);
    dot_matrix_set_command(self, 0xF, 0);

    for (int i = 0; i < 8; i++){
        dot_matrix_set_command(self, i+1, 0);
    }

    return self;
}

void dot_matrix_clean(DotMatrix* self){
    for(int i=0; i<8; i++){
        dot_matrix_set_command(self, i+1, 0);
    }
}

void dot_matrix_delete(DotMatrix* self){
    close(self->matrix_fd);
    free(self);
}

void dot_matrix_set(DotMatrix* self, int y, int x){
    if ((x > 8 || x < 0) || (y > 8 || y < 0)){
        puts("Out of borders");
        exit(1);
    }
    int Y[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
    int X[] = {0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
    dot_matrix_set_command(self, Y[y], X[x]);
}

PointController* point_controller_new(char* spi_dev_file, int speed,int red_id, int blue_id, int white_id, int black_id){
    PointController* self = (PointController*)malloc(sizeof(PointController));
    self->status = 1;
    self->Matrix = dot_matrix_create(spi_dev_file, speed);
    self->UpButton = gpio_open(red_id, "in", "rising");
    self->RightButton = gpio_open(blue_id, "in", "rising");
    self->DownButton = gpio_open(white_id, "in", "rising");
    self->LeftButton = gpio_open(black_id, "in", "rising");
    self->x = 0;
    self->y = 0;
    pthread_t up_thread;
    pthread_t right_thread;
    pthread_t down_thread;
    pthread_t left_thread;

    pthread_create(&up_thread, NULL, point_controller_move_up, self);
    pthread_create(&right_thread, NULL, point_controller_move_right, self);
    pthread_create(&down_thread, NULL, point_controller_move_down, self);
    pthread_create(&left_thread, NULL, point_controller_move_left, self);
    dot_matrix_set(self->Matrix, 0, 0);
    return self;
}

void point_controller_delete(PointController* self){
    gpio_close(self->UpButton);
    gpio_close(self->RightButton);
    gpio_close(self->DownButton);
    gpio_close(self->LeftButton);
    dot_matrix_delete(self->Matrix);
    free(self);
}

void* point_controller_move_up(void* args){
    PointController* self = (PointController*)args;
    while(self->status == 1){
        int value = gpio_poll(self->RightButton, 10);
        if(value == 1){
            dot_matrix_clean(self->Matrix);
            dot_matrix_set(self->Matrix, self->x, ++self->y);
        }
    }
}

void* point_controller_move_right(void* args){
    PointController* self = (PointController*)args;
    while(self->status == 1){
        int value = gpio_poll(self->UpButton, 10);
        if(value == 1){
            dot_matrix_clean(self->Matrix);
            dot_matrix_set(self->Matrix, ++self->x, self->y);
        }
    }
}

void* point_controller_move_down(void* args){
    PointController* self = (PointController*)args;
    while(self->status == 1){
        int value = gpio_poll(self->DownButton, 10);
        if(value == 1){
            dot_matrix_clean(self->Matrix);
            dot_matrix_set(self->Matrix, self->x, --self->y);
        }
    }
}

void* point_controller_move_left(void* args){
    PointController* self = (PointController*)args;
    while(self->status == 1){
        int value = gpio_poll(self->LeftButton, 10);
        if(value == 1){
            dot_matrix_clean(self->Matrix);
            dot_matrix_set(self->Matrix, --self->x, self->y);
        }
    }
}