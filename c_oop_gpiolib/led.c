#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "oop_my_gpio.h"


int main(int argc, char *argv[]){
    Gpio *pin11 = gpio_open(1, "out", "none");
	gpio_write(pin11, 1);
	sleep(2);
	gpio_write(pin11, 0);
	gpio_close(pin11);
	return 0;
}