#include <stdint.h>

#ifndef DEBUG
#include <xiomodule.h>
#include "fifo.h"
#endif

#include "build_tx.h"

static int bit_p = 32;
static uint32_t cur_buf;

walsh_t walsh_data[] = {    0xFFFF, 0xAAAA, 0xCCCC, 0x9999,
			    0xF0F0, 0xA5A5, 0xC3C3, 0x9696,
			    0xFF00, 0xAA55, 0xCC33, 0x9966,
			    0xF00F, 0xA55A, 0xC33C, 0x9669 };

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

    crc = crc8_update(crc, (header & 0xff));
    crc = crc8_update(crc, ((header >> 8) & 0xff));
    crc = crc8_update(crc, ((header >> 16) & 0xff));
    
    return crc;
}

#define Z(val) ((seed >> (32 - val)) & 1)
static uint32_t update_scrambler(uint32_t seed) {
    return (seed >> 1) | ((Z(11) ^ Z(31) ^ Z(32)) << 31);
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

static void put_sf_0(tx_rate_t rate) {
    switch (rate) {
        case r_sf_64:
            put_n_bits(BIT_MAP_32_0, 32);
            put_n_bits(BIT_MAP_32_0, 32);
            break;
        case r_sf_32:
            put_n_bits(BIT_MAP_32_0, 32);
            break;
        case r_sf_16:
            put_n_bits(BIT_MAP_32_0 >> 16, 16);
            break;
        case r_sf_8:
            put_n_bits(BIT_MAP_32_0 >> 24, 8);
            break;
    }
}

static void put_sf_1(tx_rate_t rate) {
    switch (rate) {
        case r_sf_64:
            put_n_bits(BIT_MAP_32_1, 32);
            put_n_bits(BIT_MAP_32_1, 32);
            break;
        case r_sf_32:
            put_n_bits(BIT_MAP_32_1, 32);
            break;
        case r_sf_16:
            put_n_bits(BIT_MAP_32_1 >> 16, 16);
            break;
        case r_sf_8:
            put_n_bits(BIT_MAP_32_1 >> 24, 8);
            break;
    }
}

static void put_bitmap_32(uint32_t code) {
    int i;
    for (i = 0; i < 32; i++) {
	if (code & (1 << (31 - i))) put_sf_1(r_sf_8);
	else put_sf_0(r_sf_8);
    }
}

void build_tx_preamble(void) {
    put_bitmap_32(PREAMBLE_1);
    put_bitmap_32(PREAMBLE_2);
}

void build_tx_sfd(plcp_header_t *header_info) {
    if (!header_info->use_ri) {
	put_bitmap_32(SFD_1);
	put_bitmap_32(SFD_2);
	return;
    }

    switch (header_info->data_rate) {
	case r_sf_64:
	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);
	    /* 12 * sf_8 Chips  = 3 * sf_32 */
	    put_sf_0(r_sf_32);
	    put_sf_0(r_sf_32);
	    put_sf_0(r_sf_32);
	    break;
	case r_sf_32:
	    put_sf_0(r_sf_16);
	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);
	    put_sf_0(r_sf_32);
	    put_sf_0(r_sf_32);
	    put_sf_0(r_sf_16);
	    break;
	case r_sf_16:
	    put_sf_0(r_sf_32);
	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);
	    put_sf_0(r_sf_32);
	    put_sf_0(r_sf_32);
	    break;
	case r_sf_8:
	    put_sf_0(r_sf_32);
	    put_sf_0(r_sf_16);
	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);
	    put_sf_0(r_sf_32);
	    put_sf_0(r_sf_16);
	    break;
    }
}

static void modulate(uint32_t data, tx_rate_t rate) {
    /* Bit order isn't clear - send LSB first CHECK THIS */
    int i, j;
    walsh_t walsh_code;

    /* Symbol size is 4 bits */
    for (i = 0; i < 32; i += SYM_SIZE) {
	walsh_code = walsh_data[(data >> i) & SYM_MASK];
	for (j = 0; j < WALSH_SIZE; j++) {
	    if (walsh_code & (1 << j)) put_sf_1(rate);
	    else put_sf_0(rate);
	}
    }
}

/* Table 82 */
void build_tx_plcp_header(plcp_header_t *header_info) {
    uint32_t header_bits = 0;

    header_bits |= header_info->data_rate	<< HDR_DR_SHIFT;
    header_bits |= header_info->pilot_info	<< HDR_PI_SHIFT;
    header_bits |= header_info->burst_mode	<< HDR_BM_SHIFT;
    header_bits |= header_info->scrambler_seed	<< HDR_SS_SHIFT;
    header_bits |= header_info->PDSU_length	<< HDR_LEN_SHIFT;
    header_info->crc8 = crc8(header_bits);
    header_bits |= header_info->crc8		<< HDR_CRC_SHIFT;

    build_tx_preamble();
    build_tx_sfd(header_info);

    if (header_info->use_ri) modulate(header_bits, header_info->data_rate);
    else modulate(header_bits, r_sf_64);
}

/* Implement this as a separate FIFO for efficiency */
void build_tx_payload(void) {}

void tx_trigger(void) {
    flush();
    fifo_trigger();
}
