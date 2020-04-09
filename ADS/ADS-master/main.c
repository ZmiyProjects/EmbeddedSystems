#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "i2c.h"

/* Имя специального файла I2C шины */
#define I2C_DEV_FILE "/dev/i2c-0"
/* Адрес подключения к I2С шине */
#define I2C_ADDRESS 0x49

typedef struct ads {
    int status;
    I2C* device;
    unsigned char wbuf[3];
    unsigned char rbuf[2];
    double voltage;
} ADS;

ADS* ads_new(char* device, int addr);
void ads_active_read(ADS* self);
void ads_delete(ADS* self);
double ads_get_voltage(ADS* self);


void* eternal_read(void* args);

ADS* ads_new(char* device, int addr){
    ADS* self = (ADS*)malloc(sizeof(ADS));
    self->status = 1;
    self->device = i2c_open(device, addr);
    self->wbuf[0] = 1;

    self->wbuf[1] = 0xC3;
    self->wbuf[2] = 0x03;

    i2c_write(self->device, self->wbuf, 3);

    pthread_t reader_thread;
    pthread_create(&reader_thread, NULL, eternal_read, self);
    return self;
}

void ads_active_read(ADS* self){
    self->wbuf[0] = 0;
    i2c_write(self->device, self->wbuf, 1);

    i2c_read(self->device, self->rbuf, 2);

    short val = self->rbuf[0] << 8 | self->rbuf[1];

    self->voltage = (float)val*4.096/32767.0;

}

void ads_delete(ADS* self){
    i2c_close(self->device);
    free(self);
}

double ads_get_voltage(ADS* self){
    return self->voltage;
}

void* eternal_read(void* args){
    ADS* device = (ADS*)args;
    while (device->status == 1){
        ads_active_read(device);
        printf("%f\n", ads_get_voltage(device));
        sleep(1);
    }
}

int main() {
    ADS* ads_device = ads_new(I2C_DEV_FILE, I2C_ADDRESS);
    getchar();
    ads_device->status = 0;
    /*ads_active_read(ads_device);
    printf("%f", ads_get_voltage(ads_device));*/
    ads_delete(ads_device);
    return 0;
}
