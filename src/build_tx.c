#include <stdint.h>

#ifndef DEBUG
#include <xiomodule.h>
#include "fifo.h"
#endif

#include "build_tx.h"

static uint32_t cur_buf; 

static void put_byte(uint8_t byte) {
    static int where;
    switch (where) {
	case 0 :
	    cur_buf = 0;
	    cur_buf |= byte << 24;
	    where = 1;
	    break;
	case 1 :
	    cur_buf |= byte << 16;
	    where = 2;
	    break;
	case 2 :
	    cur_buf |= byte << 8;
	    where = 3;
	    break;
	case 3 :
	    cur_buf |= byte;
	    where = 0;
	    fifo_write(cur_buf);
    }
}

void build_tx_preamble(void) {
    int i;
    for (i = 0; i < 32; i++) {
	if (PREAMBLE_1 & (1 << (31 - i))) put_byte(BIT_MAP_8_1);
	else put_byte(BIT_MAP_8_0);
    }
    for (i = 0; i < 32; i++) {
	if (PREAMBLE_2 & (1 << (31 - i))) put_byte(BIT_MAP_8_1);
	else put_byte(BIT_MAP_8_0);
    }
}

void build_tx_sfd_ri(void) {}
void build_tx_plcp_header(void) {}
void build_tx_payload(void) {}
