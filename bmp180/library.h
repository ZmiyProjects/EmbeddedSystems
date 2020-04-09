#include "i2c.h"
#include <stdlib.h>
#include <unistd.h>


#ifndef BMP180_LIBRARY_H
#define BMP180_LIBRARY_H

typedef struct bmp180 BMP180;

BMP180* bmp180_new(char* device, int addr);

void bmp_delete(BMP180* self);

int get_timing(int);

void bmp180_get_values(BMP180* self, int oss);


#endif //BMP180_LIBRARY_H