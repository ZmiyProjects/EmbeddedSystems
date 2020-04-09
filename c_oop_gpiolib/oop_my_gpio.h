#ifndef _MY_GPIO_
#define _MY_GPIO_

typedef struct Gpio Gpio;

Gpio* gpio_open(int line, const char *direction, const char *edge);

void gpio_set_direction(Gpio* self, const char *direction);

void gpio_set_edge(Gpio *self, const char *edge);

char gpio_read(Gpio *self);

char gpio_poll(Gpio *self, int timeout);

void gpio_write(Gpio *self, char value);

void gpio_close(Gpio *self);

#endif

