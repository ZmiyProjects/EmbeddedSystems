#ifndef _MY_I2C_
#define _MY_I2C_


typedef struct i2c_device{
    int fd;
    int addr;
    char* device;
} I2C;

I2C* i2c_open(char* device, int addr);

unsigned char i2c_read_reg8(I2C* self, unsigned char addr);

void i2c_write_reg8(I2C* self, unsigned char addr, unsigned char value);

void i2c_read(I2C* self, unsigned char *buf, int nbytes);

void i2c_write(I2C* self, unsigned char *buf, int nbytes);

void i2c_close(I2C* self);

#endif
