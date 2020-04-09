#include "library.h"
#include "display/oop_display.h"
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Имя специального файла I2C шины */
#define I2C_DEV_FILE "/dev/i2c-0"
/* Адрес подключения датчика к I2С шине */
#define I2C_ADDRESS 0x53

/* Константа пересчета ускорения в единицы системы СИ м/c2 */
#define CONV (9.8 * 32 / 0x2000)

/* Адреса регистров датчика */
#define DATA_FORMAT         0x31
#define BW_RATE             0x2C
#define POWER_CTL           0x2D
#define DATAX0              0x32
#define DATAX1              0x33
#define DATAY0              0x34
#define DATAY1              0x35
#define DATAZ0              0x36
#define DATAZ1              0x37

typedef struct adxl345{
    I2C* device;
    int data_format;
    int bw_rate;
    int power_ctl;
    int x0;
    int x1;
    int y0;
    int y1;
    int z0;
    int z1;

    int x_value;
    int y_value;
    int z_value;
} ADXL345;

ADXL345* adxl345_new(char* device, int addr){
    ADXL345* self = malloc(sizeof(ADXL345));
    self->device = i2c_open(device, addr);
    self->data_format = 0x31;
    self->bw_rate = 0x2C;
    self->power_ctl = 0x2D;
    self->x0 = 0x32;
    self->x1 = 0x33;
    self->y0 = 0x34;
    self->y1 = 0x35;
    self->z0 = 0x36;
    self->z1 = 0x37;

    i2c_write_reg8(self->device, self->power_ctl, 0x00);
    i2c_write_reg8(self->device, self->bw_rate, 0x00);
    return self;
}

void adxl345_delete(ADXL345* self){
    i2c_close(self->device);
}

void adxl345_init(ADXL345* self, int power_ctl, int  bw_rate, int data_format){
    /* Устанавливаем режимы работы датчика */
    i2c_write_reg8(self->device, self->bw_rate, bw_rate);
    /* Set range  +-16 g */
    i2c_write_reg8(self->device, self->data_format, data_format);
    /* Measurement on auto sleep mode on */
    i2c_write_reg8(self->device, self->power_ctl, power_ctl);
}


/* Функция чтения ускорения */
int adxl345_read_a(ADXL345* self, int DATA0, int DATA1){
    short a;
    unsigned char buf[2] = {0};

    buf[0] = i2c_read_reg8(self->device, DATA0);
    buf[1] = i2c_read_reg8(self->device, DATA1);

    a = (buf[1] << 8) | buf[0];

    return a;
}

void adxl345_read_xyz(ADXL345* self){
    self->x_value = adxl345_read_a(self, self->x0, self->x1);
    self->y_value = adxl345_read_a(self, self->y0, self->y1);
    self->z_value = adxl345_read_a(self, self->z0, self->z1);
}

void display_adxl(Display* disp,  int value){
    char buffer[8];
    bool points[4] = {false, true, false, false};
    sprintf(buffer, "%+05.1f", CONV * value);
    puts(buffer);
    unsigned int mark_x = buffer[0] == '-' ? 11 : 10;
    unsigned char values_x[] = {mark_x, buffer[1] - 48, buffer[2] - 48, buffer[4] - 48};
    display_print_values(disp, values_x, points);
}

int main(int argc, char *argv[]){
    Display* disp = display_new(1, 0, 3);

    /* Открываем I2C шину для работы */
    ADXL345* device = adxl345_new(I2C_DEV_FILE, I2C_ADDRESS);
    // fd = i2c_open(I2C_DEV_FILE, I2C_ADDRESS);

    usleep(500000);
    adxl345_init(device, 0x8, 0xB, 0xA);
    /* Инициализируем регистры датчика
    i2c_write_reg8(fd, POWER_CTL, 0x00);
    i2c_write_reg8(fd, BW_RATE, 0x00);*/

    /* Устанавливаем режимы работы датчика
    i2c_write_reg8(fd, BW_RATE, 0xA);
     Set range  +-16 g
    i2c_write_reg8(fd, DATA_FORMAT, 0xb);
     Measurement on auto sleep mode on
    i2c_write_reg8(fd, POWER_CTL, 0x8);*/

    usleep(500000);

    /* Считываем значения ускорений по осям */
    adxl345_read_xyz(device);

    /* Выводим на экран значения ускорений в единицах
       системы CИ (м/c^2)*/
    printf("ax = %f; ay = %f; az = %f\n", CONV*device->x_value, CONV*device->y_value, CONV*device->z_value);
    display_adxl(disp, device->x_value);
    getchar();
    display_adxl(disp, device->y_value);
    getchar();
    display_adxl(disp, device->z_value);
    getchar();
    adxl345_delete(device);
    display_clear(disp);
    display_delete(disp);
    return 0;
}


