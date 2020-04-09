#include "library.h"

#include <stdio.h>

#include <unistd.h>

#include "i2c.h"

/* Имя специального файла I2C шины */
#define I2C_DEV_FILE "/dev/i2c-0"
/* Адрес подключения датчика к I2С шине */
#define I2C_ADDRESS  0x77

/* Режим работы датчика давления (см. таб. 3) */
#define OSS 3

typedef struct bmp180 {
    I2C* device;
    short AC1;
    short AC2;
    short AC3;
    unsigned short AC4;
    unsigned short AC5;
    unsigned short AC6;
    short B1;
    short B2;
    short MB;
    short MC;
    short MD;

    long u_pressurage;
    long u_temperature;
    long pressurage;
    long temperature;
} BMP180;




/* Функция, определяющая время (в мсек) задержки перед считыванием
   нового значения давления в зависимости от режима работы датчика
   (см. таб. 1) */
int get_timing(int oss){
    static int values[4] = {4500, 7500, 13500, 25500};
    if (oss < 0 || oss > 3){
        puts("get_timing Value error!");
        exit(1);
    }
    return values[oss];
}

// создание нового объекта датчика и калибровка
BMP180* bmp180_new(char* device, int addr){
    BMP180* self = malloc(sizeof(BMP180));
    self->device = i2c_open(device, addr);

    self->AC1 = (i2c_read_reg8(self->device, 0xAA) << 8) + i2c_read_reg8(self->device, 0xAB);
    self->AC2 = (i2c_read_reg8(self->device, 0xAC) << 8) + i2c_read_reg8(self->device, 0xAD);
    self->AC3 = (i2c_read_reg8(self->device, 0xAE) << 8) + i2c_read_reg8(self->device, 0xAF);
    self->AC4 = (i2c_read_reg8(self->device, 0xB0) << 8) + i2c_read_reg8(self->device, 0xB1);
    self->AC5 = (i2c_read_reg8(self->device, 0xB2) << 8) + i2c_read_reg8(self->device, 0xB3);
    self->AC6 = (i2c_read_reg8(self->device, 0xB4) << 8) + i2c_read_reg8(self->device, 0xB5);
    self->B1 = (i2c_read_reg8(self->device, 0xB6) << 8) + i2c_read_reg8(self->device, 0xB7);
    self->B2 = (i2c_read_reg8(self->device, 0xB8) << 8) + i2c_read_reg8(self->device, 0xB9);
    self->MB = (i2c_read_reg8(self->device, 0xBA) << 8) + i2c_read_reg8(self->device, 0xBB);
    self->MC = (i2c_read_reg8(self->device, 0xBC) << 8) + i2c_read_reg8(self->device, 0xBD);
    self->MD = (i2c_read_reg8(self->device, 0xBE) << 8) + i2c_read_reg8(self->device, 0xBF);

    return self;
}

void bmp_delete(BMP180* self){
    i2c_close(self->device);
    free(self);
}

void bmp180_get_values(BMP180* self, int oss){
    int x,y,z;
    char buf[6] = {0};
    int MSB, LSB, XLSB, timing;

    /* Открываем I2C шину для работы */
    // fd = i2c_open(I2C_DEV_FILE, I2C_ADDRESS);


    /* Читаем сырое значения температуры */
    i2c_write_reg8(self->device, 0xF4, 0x2E);
    usleep(4500);
    MSB = i2c_read_reg8(self->device, 0xF6);
    LSB = i2c_read_reg8(self->device, 0xF7);
    self->u_temperature = (MSB << 8) + LSB;

    /* Читаем сырое значение давления */
    timing = get_timing(3);
    i2c_write_reg8(self->device, 0xF4, 0x34 + (oss << 6));
    usleep(timing);
    MSB = i2c_read_reg8(self->device, 0xF6);
    LSB = i2c_read_reg8(self->device, 0xF7);
    XLSB = i2c_read_reg8(self->device, 0xF8);
    self->u_pressurage = ((MSB << 16) + (LSB << 8) + XLSB) >> (8 - oss);

    long X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12;
    long B3, B5, B6, B7, P0;
    unsigned long B4;

    X1 = ((self->u_temperature - self->AC6) * self->AC5) >> 15;
    X2 = (self->MC << 11) / (X1 + self->MD);
    B5 = X1 + X2;
    self->temperature = (B5 + 8) >> 4;

    B6 = B5 - 4000;
    X4 = (self->B2 * ((B6 * B6) >> 12)) >> 11;
    X5 = (self->AC2 * B6) >> 11;
    X6 = X4 + X5;
    B3 = (((self->AC1*4 + X6) << oss) + 2) >> 2;
    X7 = (self->AC3 * B6) >> 13;
    X8 = (self->B1 * (B6 * B6 >> 12)) >> 16;
    X9 = ((X7 + X8) + 2) >> 2;
    B4 = (self->AC4 * (unsigned long)(X9 + 32768)) >> 15;
    B7 = ((unsigned long)self->u_pressurage - B3) * (50000 >> oss);
    P0 = B7 < 0x80000000 ? (B7 * 2) / B4 : (B7 / B4) * 2;

    X10 = (P0 >> 8) * (P0 >> 8);
    X11 = (X10 * 3038) >> 16;
    X12 = (-7357 * P0) >> 16;
    self->pressurage = P0 + ((X11 + X12 + 3791) >> 4);
}


int main(int argc, char *argv[]){
    BMP180* device = bmp180_new(I2C_DEV_FILE, I2C_ADDRESS);
    bmp180_get_values(device, 3);
    printf("P = %d Pa (T = %.1f *C)\n",(int)device->pressurage, (float)(device->temperature / 10));
    bmp_delete(device);
    return 0;
}