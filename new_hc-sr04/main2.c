#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h> 
#include <time.h> 
#include <unistd.h> 
#include <stdbool.h> 
#include "display/my_gpio/oop_my_gpio.h" 
#include "display/oop_display.h"
/* Максимальное число итераций 
обращений к линии */
#define _MAX_ 10000
/* Скорость звука */
#define SOUND_SPEED 340.29

long long get_time(void);

void udelay(long long ns);

typedef struct range_finder{
    Gpio *trigger;
    Gpio *echo;
    double distance;
} RangeFinder;
typedef struct dht11{
	Gpio* dht;
	double temperature;
	double humidity;
	// int data[5];
} DHT11;
DHT11* dht11_new(int line){
	DHT11* self = (DHT11*)malloc(sizeof(DHT11));
	self->dht = gpio_open(line, "out", "none");
	self->temperature = 0;
	self->humidity = 0;
	return self;
}
int dht11_calculate(DHT11* self, long long max_iter){
	int data[5] = {0, 0, 0, 0, 0};
    long long t;
    struct timespec tspec;
    char c;
    int i;
    int k;
    /* Формируем зарос к датчику */
    // gpio_open(DATA, "out", "none");
    gpio_write(self->dht, 0);
    /* Притягиваем DATA к 0 на 18 мс (18000000 
нс) */
    udelay(18000000LL);
    /* Притягиваем DATA к 1 на 30 мкс (30000 
нс) */
    gpio_write(self->dht, 1);
    udelay(30000LL);
    /* Меняем направление работы шины 
для приема данных от датчика */
    gpio_set_direction(self->dht, "in");
    for(k = 0; k < max_iter && gpio_read(self->dht) == 0; k++);
    for(k = 0; k < max_iter && gpio_read(self->dht) == 1; k++);
    for(i = 0; i < 40; i++){
        for(k = 0; k < max_iter && gpio_read(self->dht) == 0; k++);
        t = get_time();
        for(k = 0; k < max_iter && gpio_read(self->dht) == 1; k++);
        t = get_time() - t;
        if(60000LL < t && t < 800000LL){
            /* Если время импульса между 
60 и 80 мкс (то есть ~70 мкс),
               то считаем, что это 
единица.*/
            data[i / 8] |= (1 << (7 - (i % 8)));
        }else if(t < 10000LL || 80000LL < t){
           printf("Ошибка чтения данных!\n");
           return 1;
        }
    }
    /* Проверяем корректность 
данных */
    if(data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)){
        printf("H = %d.%d %% T = %d.%d *C\n", data[0], data[1], data[2], data[3]);
		char buffer[12];
                if(data[1] <= 9){
                    data[1] *= 10;
                }
		sprintf(buffer, "%02d.%02d", data[0], data[1]);
		self->humidity = strtod(buffer, NULL);
                if(data[3] <= 9){
                    data[3] *= 10;
                }
                sprintf(buffer, "%02d.%02d", data[2], data[3]);
		self->temperature = strtod(buffer, NULL);
		return 0;
    }else{
        printf("Данные некорректны!\n" );
        return 1;
    }
}

void dht11_delete(DHT11* self);

void dht11_delete(DHT11* self){
	gpio_close(self->dht);
	free(self);
}
void udelay(long long ns){
    long long start_time = get_time();
    long long end_time;
    do {
        end_time = get_time();
    } while(end_time - start_time < ns);
}
RangeFinder* range_finder_new(int trigger, int echo){
    RangeFinder *self = (RangeFinder*)malloc(sizeof(RangeFinder));
    self->trigger = gpio_open(trigger, "out", NULL);
    self->echo = gpio_open(echo, "in", NULL);
    self->distance = 0;
    return self;
}
void range_finder_delete(RangeFinder *self){
    gpio_close(self->trigger);
    gpio_close(self->echo);
    free(self);
}
/* Функция возвращает текущее 
время в микросекундах */ long long get_time(){
    /* Структура с информацией о 
текущем времени */
    struct timespec tspec;
    /* Запрашиваем текущее время */
    clock_gettime(CLOCK_REALTIME, &tspec);
    return tspec.tv_nsec;
}
void range_finder_calculate_distance(RangeFinder *self, long max_iter){
    long long int t = 0L;
    int k;
    /* Инициализируем датчик, 
притягивая TRIGGER контакт к нулю.
       Если не сделать паузу после 
этого, то иногда происходит ошибка
       работы датчика. */
    gpio_write(self->trigger, 0);
    usleep(500000);
    /* Инициируем излучение 8 пачек 
ультразвуковых сигналов, 
притягивая
       контакт TRIGGER к 1 на 10 мкс */
    gpio_write(self->trigger, 1);
    usleep(10);
    gpio_write(self->trigger, 0);
    /* Ожидаем, когда входном сигнале ECHO 
будет логический ноль. */
    for(k = 0; k < max_iter && gpio_read(self->echo) == 0; k++);
    t = get_time();
    /* Ждем эхо-сигнал */
    for(k = 0; k < max_iter && gpio_read(self->echo) == 1; k++);
    t = get_time() - t;
    self->distance = ((1.0 * t / 1000000000.0) * SOUND_SPEED) / 2.0;
}
int main() {
        int code = 1;
	DHT11* temp = dht11_new(12);
        Display* device = display_new(1, 0, 3);
        char buffer[8];
        bool points[4] = {false, true, false, false};
        // unsigned char values[4] = {5, 5, 5, 5};
        do{
          code = dht11_calculate(temp, _MAX_);
          usleep(100);
        } while ( code != 0 );
            printf("T = %05.2f : H = %05.2f\n", temp->temperature, temp->humidity);
            sprintf(buffer, "%05.2f", temp->temperature);
            unsigned char values_t[] = {buffer[0] - 48, buffer[1] - 48, buffer[3] - 48, buffer[4] - 48};
            display_print_values(device, values_t, points);
            getchar();
            sprintf(buffer, "%05.2f", temp->humidity);
            unsigned char values_hu[] = {buffer[0] - 48, buffer[1] - 48, buffer[3] - 48, buffer[4] - 48};
            display_print_values(device, values_hu, points);
            getchar();
	dht11_delete(temp);
        display_delete(device);
    return 0;
}
