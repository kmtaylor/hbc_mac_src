#include <stdint.h>
#include <string.h>

#ifndef DEBUG
#include <xiomodule.h>
#include "fifo.h"
#include "scrambler.h"
#include "mem.h"
#include "lcd.h"
#include "interrupt.h"
#else
extern void fifo_write(uint32_t data);
extern uint32_t scrambler_read(void);
extern void scrambler_reseed(int seed);
extern uint32_t mem_read(void);
extern void fifo_trigger(void);
extern void fifo_reset(void);
extern void fifo_modulate_sf(uint8_t sf);
extern void fifo_write_size(uint8_t size);
#endif

#include "build_tx.h"

uint8_t crc8_update(uint8_t crc, uint8_t data) {
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

    crc = crc8_update(crc, header >> 0);
    crc = crc8_update(crc, header >> 8);
    crc = crc8_update(crc, header >> 16);
    
    return crc;
}

static void fifo_set_rate(tx_rate_t rate) {
    fifo_modulate_sf(rate);
    switch (rate) {
	case r_sf_64 :
	    fifo_write_size(32);
	    break;
	case r_sf_32 :
	    fifo_write_size(32);
	    break;
	case r_sf_16 :
	    fifo_write_size(16);
	    break;
	case r_sf_8 :
	    fifo_write_size(8);
	    break;
    }
}

static void put_bitmap_32(uint32_t code) {
    int i;
    fifo_set_rate(r_sf_8);
    for (i = 0; i < 32; i++) {
	if (code & (1 << (31 - i))) fifo_write(BIT_MAP_32_1);
	else fifo_write(BIT_MAP_32_0);
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
	    fifo_write_size(32);
	    fifo_write(BIT_MAP_32_0);
	    fifo_write(BIT_MAP_32_0);
	    fifo_write(BIT_MAP_32_0);
	    break;
	case r_sf_32:
	    fifo_write_size(16);
	    fifo_write(BIT_MAP_32_0);

	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);

	    fifo_write_size(32);
	    fifo_write(BIT_MAP_32_0);
	    fifo_write(BIT_MAP_32_0);
	    fifo_write_size(16);
	    fifo_write(BIT_MAP_32_0);
	    break;
	case r_sf_16:
	    fifo_write_size(32);
	    fifo_write(BIT_MAP_32_0);

	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);

	    fifo_write_size(32);
	    fifo_write(BIT_MAP_32_0);
	    fifo_write(BIT_MAP_32_0);
	    break;
	case r_sf_8:
	    fifo_write_size(32);
	    fifo_write(BIT_MAP_32_0);
	    fifo_write_size(16);
	    fifo_write(BIT_MAP_32_0);

	    put_bitmap_32(SFD_1);
	    put_bitmap_32(SFD_2);

	    fifo_write_size(32);
	    fifo_write(BIT_MAP_32_0);
	    fifo_write_size(16);
	    fifo_write(BIT_MAP_32_0);
	    break;
    }
}

/* Table 82 */
void build_tx_plcp_header(plcp_header_t *header_info) {
    uint32_t header_bits = 0;

    fifo_reset();

    header_bits |= header_info->data_rate	<< HDR_DR_SHIFT;
    header_bits |= header_info->pilot_info	<< HDR_PI_SHIFT;
    header_bits |= header_info->burst_mode	<< HDR_BM_SHIFT;
    header_bits |= header_info->scrambler_seed	<< HDR_SS_SHIFT;
    header_bits |= header_info->PDSU_length	<< HDR_LEN_SHIFT;
    header_info->crc8 = crc8(header_bits);
    header_bits |= header_info->crc8		<< HDR_CRC_SHIFT;

    build_tx_preamble();
    build_tx_preamble();
    build_tx_preamble();
    build_tx_preamble();
    build_tx_sfd(header_info);

    if (header_info->use_ri) {
	fifo_set_rate(header_info->data_rate);
	fifo_modulate(header_bits);
    } else {
	fifo_set_rate(r_sf_64);
	fifo_modulate(header_bits);
    }
}

void build_tx_payload(plcp_header_t *header_info) {
    uint32_t data;
    uint8_t num_words = header_info->PDSU_length / 4;
    if (header_info->PDSU_length % 4) num_words++;

    fifo_writing = 1;

    fifo_set_rate(header_info->data_rate);
    scrambler_reseed(header_info->scrambler_seed);
    while (num_words) {
	data = mem_read();
	data ^= scrambler_read();
	fifo_modulate(data);
	num_words--;
    }

    fifo_writing = 0;

    fifo_flush();
    fifo_trigger();
}
