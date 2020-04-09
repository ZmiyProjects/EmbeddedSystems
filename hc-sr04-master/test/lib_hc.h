#ifndef TEST_LIB_HC_H
#define TEST_LIB_HC_H

typedef struct Gpio {
    int value_fd;
    int direction_fd;
    int edge_fd;
    int sysfs_id;
} Gpio;

Gpio* gpio_open(int line, const char *direction, const char *edge);

void gpio_set_direction(Gpio* self, const char *direction);

void gpio_set_edge(Gpio *self, const char *edge);

char gpio_read(Gpio *self);

char gpio_poll(Gpio *self, int timeout);

void gpio_write(Gpio *self, char value);

void gpio_close(Gpio *self);

typedef struct range_finder{
    double distance;
    Gpio* echo;
    Gpio* trigger;
} RangeFinder;

/* Функция возвращает текущее время в микросекундах */
long long get_time(void);

RangeFinder* range_finder_new(int echo, int trigger);

void range_finder_delete(RangeFinder* self);

float range_finder_calculate_distance(RangeFinder* self, unsigned long max_iter);

#endif //TEST_LIB_HC_H
