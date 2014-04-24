#include <stdint.h>

#ifndef DEBUG
#include <xiomodule.h>
#include "fifo.h"
#endif

#include "build_tx.h"

static int use_ri;
static int bit_p = 32;
static uint32_t cur_buf;
static tx_rate_t tx_rate;

static void flush(void) {
    fifo_write(cur_buf);
    cur_buf = 0;
    bit_p = 32;
}

static void put_n_bits(uint32_t bits, int num) {
    uint32_t mask;

    /* Handle wrapping to next 32 bits */
    if (num > bit_p) {
	num = num - bit_p;
	mask = ((1 << bit_p) - 1) << num;
	cur_buf |= (bits & mask) >> num;
	flush();
    }

    bit_p -= num;
    mask = (num == 32) ? -1 : (1 << num) - 1;
    cur_buf |= (bits & mask) << bit_p;

    if (bit_p == 0) flush();
}

static void put_32bit_gold_code(uint32_t code) {
    int i;
    for (i = 0; i < 32; i++) {
	if (code & (1 << (31 - i))) PUT_SF8_1;
	else PUT_SF8_0;
    }
}

void build_tx_preamble(void) {
    put_32bit_gold_code(PREAMBLE_1);
    put_32bit_gold_code(PREAMBLE_2);
}

void build_tx_sfd_ri(void) {
    if (!use_ri) {
	put_32bit_gold_code(SFD_1);
	put_32bit_gold_code(SFD_2);
	return;
    }
    switch (tx_rate) {
	case r_sf_8:
	    break;
	case r_sf_4:
	    break;
	case r_sf_2:
	    break;
	case r_sf_1:
	    break;
    }
}

void build_tx_plcp_header(void) {}
void build_tx_payload(void) {}
