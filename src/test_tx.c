/* Copyright (C) 2016 Kim Taylor
 *
 * This file is part of hbc_mac.
 *
 * hbc_mac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hbc_mac is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hbc_mac.  If not, see <http://www.gnu.org/licenses/>.
 */

/* gcc -DDEBUG -Wall test_tx.c build_tx.c -o test_tx */

#include <stdio.h>
#include <stdint.h>
#include "build_tx.h"

int fifo_used;
int fifo_writing;

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
#if 1
    plcp_header_t header_info = {
	.data_rate = r_sf_8,
	.pilot_info = pilot_none,
	.use_ri = 1,
	.burst_mode = 0,
	.scrambler_seed = 0,
	.PDSU_length = 255
    };

    build_tx_plcp_header(&header_info);
    build_tx_payload(&header_info);
#endif

    printf("FIFO used: %i\n", fifo_used);
    return 0;
}
