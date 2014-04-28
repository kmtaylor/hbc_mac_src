#include <stdint.h>

#ifndef DEBUG
#include <xiomodule.h>
#include "fifo.h"
#endif

#include "build_tx.h"

static int bit_p = 32;
static uint32_t cur_buf;
static tx_rate_t tx_rate;

static void flush(void) {
    fifo_write(cur_buf);
    cur_buf = 0;
    bit_p = 32;
}

static uint8_t crc8_update(uint8_t crc, uint8_t data) {
    uint8_t i;

    crc = crc ^ data;
    for (i = 0; i < 8; i++) {
        if (crc & 0x01)
            crc = (crc >> 1) ^ CRC8_POLY;
        else
            crc >>= 1;
    }

    return crc;
}

static uint8_t crc8(uint32_t header) {
    uint8_t crc = CRC8_INIT;

    crc8_update(crc, (header & 0xff));
    crc8_update(crc, ((header >> 8) & 0xff));
    crc8_update(crc, ((header >> 16) & 0xff));
    
    return crc;
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

static void put_bitmap_32(uint32_t code) {
    int i;
    for (i = 0; i < 32; i++) {
	if (code & (1 << (31 - i))) PUT_SF8_1;
	else PUT_SF8_0;
    }
}

void build_tx_preamble(void) {
    put_bitmap_32(PREAMBLE_1);
    put_bitmap_32(PREAMBLE_2);
}

void build_tx_sfd(void) {
    put_bitmap_32(SFD_1);
    put_bitmap_32(SFD_2);
}

void build_tx_ri(void) {
    switch (tx_rate) {
	case r_sf_8:
	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);
	    put_n_bits(0, 12);
	    break;
	case r_sf_4:
	    put_n_bits(0, 2);
	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);
	    put_n_bits(0, 10);
	    break;
	case r_sf_2:
	    put_n_bits(0, 4);
	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);
	    put_n_bits(0, 8);
	    break;
	case r_sf_1:
	    put_n_bits(0, 6);
	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);
	    put_n_bits(0, 6);
	    break;
    }
}

/* Table 82 */
void build_tx_plcp_header_drf(plcp_header_t *header_info) {
    uint32_t header_bits;

    header_bits = header_info->data_rate;
    header_bits |= header_info->pilot_info << 3;
    header_bits |= header_info->burst_mode << 8;
    header_bits |= header_info->scrambler_seed << 11;
    header_bits |= header_info->PDSU_length << 16;
    header_bits |= crc8(header_bits) << 24;
}

void build_tx_plcp_header_ri(plcp_header_t *header_info) {
}

void build_tx_payload(void) {}

void tx_trigger(void) {
    flush();
    fifo_trigger();
}
