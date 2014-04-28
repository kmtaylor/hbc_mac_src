/* gcc -DDEBUG -Wall test_tx.c build_tx.c -o test_tx */

#include <stdio.h>
#include <stdint.h>
#include "build_tx.h"

int fifo_used;

void fifo_write(uint32_t data) {
    printf("%08X\n", data);
    fifo_used++;
}

void fifo_trigger(void) {
}

int main (void) {
    plcp_header_t header_info = {
	.data_rate = r_sf_64,
	.pilot_info = pilot_none,
	.use_ri = 1,
	.burst_mode = 0,
	.scrambler_seed = 0,
	.PDSU_length = 128
    };

    //build_tx_preamble();
    //build_tx_sfd(&header_info);
    build_tx_plcp_header(&header_info);
    printf("CRC: %02X\n", header_info.crc8);
    printf("FIFO used: %i\n", fifo_used);
    return 0;
}
