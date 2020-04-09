#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "oop_my_gpio.h"
#include "oop_display.h"
#include "oop_hc-sr04.h"


int main(int argc, char *argv[]){
    RangeFinder* hc = range_finder_new(12, 11);
    float distance = range_finder_calculate_distance(hc, 100000);
    printf("%05.2f\n", distance);
    // range_finder_delete(hc);
    return 0;
}

