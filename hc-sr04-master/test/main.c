#include <stdio.h>
#include "lib_hc.h"

int main() {
    RangeFinder* hc = range_finder_new(12, 11);
    float distance = range_finder_calculate_distance(hc, 100000);
    printf("%05.2f\n", distance);
    range_finder_delete(hc);
    return 0;
}