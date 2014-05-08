/* gcc -DDEBUG -Wall test_tx.c build_tx.c -o test_tx */

#include <stdio.h>
#include <stdint.h>
#include "build_tx.h"

int fifo_used;
int writing;

void fifo_write(uint32_t data) {
    printf("%08X\n", data);
    fifo_used++;
}

void fifo_trigger(void) {
}

void fifo_reset(void) {
}

uint32_t scrambler_read(void) {
    return 0x69540152;
}

uint32_t mem_read(void) {
    return 0x12345678;
}

void scrambler_reseed(int seed) {
}

int main (void) {
#if 0
    plcp_header_t header_info = {
	.data_rate = r_sf_64,
	.pilot_info = pilot_none,
	.use_ri = 0,
	.burst_mode = 0,
	.scrambler_seed = 0,
	.PDSU_length = 100
    };

    build_tx_plcp_header(&header_info);
    build_tx_payload(&header_info);
#endif

    put_n_bits(0x12345678, 32);
    printf("FIFO used: %i\n", fifo_used);
    return 0;
}
