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

#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "interrupt.h"
#include "fifo.h"
#include "extract_rx.h"
#include "scrambler.h"
#include "build_tx.h"
#include "mem.h"

static uint32_t rd_buf_addr = MEM_RX_BUF;
static uint32_t wr_buf_addr = MEM_RX_BUF;
static uint32_t wr_bytes; 

static volatile int packet_ready;
static uint32_t rd_packet_addr = MEM_RX_BUF;
static uint32_t wr_packet_addr;
static uint32_t rd_packet_header;
static uint32_t rd_packet_marker;

/* Should be called with interrupts disabled */
static void read_headers(void) {
    mem_set_rd_p(rd_packet_addr);
    rd_packet_marker = mem_read();
    rd_packet_header = mem_read();
}

static void inc_rd_buf(void) {
    rd_buf_addr += 4;
    if (rd_buf_addr == MEM_SIZE) rd_buf_addr = MEM_RX_BUF; 
}

static void inc_wr_buf(void) {
    wr_buf_addr += 4;
    if (wr_buf_addr == MEM_SIZE) wr_buf_addr = MEM_RX_BUF;
}

int rx_packet_ready(void) {
    read_headers();
    rd_buf_addr = rd_packet_addr;
    return packet_ready;
}

uint8_t rx_packet_length(void) {
    return HDR_READ(rd_packet_header, LEN);
}

uint32_t rx_bytes_read(void) {
    return rd_packet_marker - 8;
}

int rx_check_crc_ok(void) {
    uint8_t crc = CRC8_INIT;

    crc = crc8_update(crc, rd_packet_header >> 0);
    crc = crc8_update(crc, rd_packet_header >> 8);
    crc = crc8_update(crc, rd_packet_header >> 16);
    crc = crc8_update(crc, rd_packet_header >> 24);

    return crc ? 0 : 1;
}

/* Should be called with interrupts disabled */
uint32_t rx_read(void) {
    mem_set_rd_p(rd_buf_addr);
    inc_rd_buf();
    return mem_read();
}

uint32_t rx_read_addr(void) {
    return rd_packet_addr; 
}

uint32_t rx_write_addr(void) {
    return wr_packet_addr;
}

void rx_packet_next(void) {
    if (rd_packet_marker == -1) return;

    /* 8 Extra bytes for packet marker and header */
    rd_packet_addr += rd_packet_marker;

    packet_ready--;
}

static enum { packet_init, packet_reading } rx_state = packet_init;

static void rx_data_int_func(void) {
    static uint32_t val;

    mem_set_wr_p(wr_buf_addr);
    if ((rx_state == packet_init) && IRQ_FLAG_SET(IRQ_RX_DATA_READY)) {
	wr_packet_addr = wr_buf_addr;
	val = fifo_read();
	/* Leave space for packet marker */
	mem_write(-1);
	mem_write(val);
	inc_wr_buf();
	inc_wr_buf();
	scrambler_reseed(HDR_READ(val, SS));
	rx_state = packet_reading;
	wr_bytes += 8;
    }

    while (IRQ_FLAG_SET(IRQ_RX_DATA_READY)) {
	val = fifo_read();
	mem_write(val ^ scrambler_read());
	inc_wr_buf();
	wr_bytes += 4;
    }
}

static void rx_ready_int_func(void) {
    if (IRQ_FLAG_SET(IRQ_RX_DATA_READY)) {
	/* If there is any data left, read out the last bytes */
	rx_data_int_func();
    }

    if (wr_bytes) {
	/* Write packet marker */ 
	mem_set_wr_p(wr_packet_addr);
	mem_write(wr_bytes);

	rx_state = packet_init;
	packet_ready++;
	wr_bytes = 0;
    }

    GPO_SET(HBC_RX_PKT_ACK);
    GPO_CLEAR(HBC_RX_PKT_ACK);
}

DECLARE_HANDLER(INT(IRQ_RX_FIFO_ALMOST_FULL), rx_data_int_func);
DECLARE_HANDLER(INT(IRQ_RX_PKT_READY), rx_ready_int_func);

void rx_enable(void) {
    GPO_SET(HBC_RX_ENABLE);
}

void rx_disable(void) {
    GPO_CLEAR(HBC_RX_ENABLE);
}

void rx_init(void) {
    ADD_INTERRUPT_HANDLER(INT(IRQ_RX_FIFO_ALMOST_FULL));
    ADD_INTERRUPT_HANDLER(INT(IRQ_RX_PKT_READY));
}
