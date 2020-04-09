#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <limits.h>
#include <unistd.h>

// #include "my_i2c.h"

/* Функция, инициализирующая I2C шину */
I2C* i2c_open(char *device, int addr){
    I2C* self = malloc(sizeof(I2C));
    self->addr = addr;
    /* Открываем специальный файл для работы с I2C шиной */
    if ((self->fd = open(device, O_RDWR)) < 0){
        perror("i2c_open() failed!");
        exit(1);
    }

    /* Устанавливаем адрес подключения устройства к I2C шине */
    if (ioctl(self->fd, I2C_SLAVE, self->addr) < 0){
        perror("i2c_open() failed!");
        exit(1);
    }

    return self;
}

/* Чтение регистра */
unsigned char i2c_read_reg8(I2C* self, unsigned char addr){
    unsigned char c = 0;

    if(write(self->fd, &addr, 1) != 1){
        fprintf(stderr, "i2c_getc() failed! Error writing byte\n");
    }

    if(read(self->fd, &c, 1) != 1){
        fprintf(stderr, "Error reading byte\n");
    }

    return c;
}

/* Запись регистра */
void i2c_write_reg8(I2C* self, unsigned char addr, unsigned char value){
    unsigned char buf[2] = {addr, value};

    if(write(self->fd, buf, 2) != 2){
        fprintf(stderr, "i2c_putc() failed! Error writing bytes\n");
    }
}

/* Чтение N байт */
void i2c_read(I2C* self, unsigned char *buf, int nbytes){
    if(read(self->fd, buf, nbytes) != nbytes){
        fprintf(stderr, "i2c_read() failed! Error reading bytes\n");
    }
}

/* Запись N байт */
void i2c_write(I2C* self, unsigned char *buf, int nbytes){
    if(write(self->fd, buf, nbytes) != nbytes){
        fprintf(stderr, "i2c_write() failed! Error writing bytes\n");
    }
}

/* Завершение работы с шиной */
void i2c_close(I2C* self){
    close(self->fd);
    free(self);
}