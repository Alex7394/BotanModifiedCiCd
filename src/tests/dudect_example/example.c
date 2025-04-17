#define DUDECT_IMPLEMENTATION
#include "dudect.h"
#include <string.h>
#include <stdint.h>

#define INPUT_SIZE 16

static uint8_t input_data[INPUT_SIZE];

void prepare_inputs(uint8_t *data, uint8_t class) {
    if (class == 0) {
        memset(data, 0x00, INPUT_SIZE);
    } else {
        memset(data, 0xFF, INPUT_SIZE);
    }
}

uint8_t do_one_computation(uint8_t *data) {
    volatile uint8_t sum = 0;
    for (size_t i = 0; i < INPUT_SIZE; i++) {
        sum += data[i];
    }
    return sum;
}

int main(void) {
    dudect_config_t config;
    dudect_default_config(&config);
    dudect_main(&config);
    return 0;
}
