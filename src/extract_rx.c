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

static int packet_ready;
static uint32_t rd_packet_addr = MEM_RX_BUF;
static uint32_t wr_packet_addr;
static uint32_t rd_packet_header;
static uint32_t rd_packet_marker;

static void read_headers(void) {
    mem_set_rd_p(rd_packet_addr + 1);
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
    return rd_packet_marker;
}

int rx_check_crc_ok(void) {
    uint8_t crc = CRC8_INIT;

    crc = crc8_update(crc, rd_packet_header >> 0);
    crc = crc8_update(crc, rd_packet_header >> 8);
    crc = crc8_update(crc, rd_packet_header >> 16);
    crc = crc8_update(crc, rd_packet_header >> 24);

    return crc ? 0 : 1;
}

uint32_t rx_read(void) {
    mem_set_rd_p(rd_buf_addr);
    inc_rd_buf();
    return mem_read();
}

void rx_packet_next(void) {
    uint8_t bytes, words;

    /* 8 Extra bytes for packet marker and header */
    rd_packet_addr += rx_bytes_read() + 8;

    packet_ready--;
}

static enum { packet_init, packet_reading } rx_state = packet_init;

static void rx_data_int_func(void) {
    static uint32_t val;

    mem_set_wr_p(wr_buf_addr);
    if (rx_state == packet_init) {
	wr_bytes = 0;
	wr_packet_addr = wr_buf_addr;
	val = fifo_read();
	/* Leave space for packet marker */
	mem_write(val);
	mem_write(val);
	inc_wr_buf();
	inc_wr_buf();
	scrambler_reseed(HDR_READ(val, SS));
	rx_state = packet_reading;
    }

    while (IRQ_FLAG_SET(IRQ_RX_DATA_READY)) {
	val = fifo_read();
	mem_write(val ^ scrambler_read());
	inc_wr_buf();
	wr_bytes += 4;
    }
}

static void rx_ready_int_func(void) {
    uint32_t tmp;

    if (IRQ_FLAG_SET(IRQ_RX_DATA_READY)) {
	/* If there is any data left, read out the last bytes */
	rx_data_int_func();
    }

    /* Write packet marker */ 
    tmp = mem_get_wr_p();
    mem_set_wr_p(wr_packet_addr);
    mem_write(wr_bytes);
    mem_set_wr_p(tmp);

    rx_state = packet_init;
    GPO_SET(HBC_RX_PKT_ACK);
    GPO_CLEAR(HBC_RX_PKT_ACK);
    packet_ready++;
}

DECLARE_HANDLER(INT(IRQ_RX_FIFO_ALMOST_FULL), rx_data_int_func);
DECLARE_HANDLER(INT(IRQ_RX_PKT_READY), rx_ready_int_func);

void rx_init(void) {
    ADD_INTERRUPT_HANDLER(INT(IRQ_RX_FIFO_ALMOST_FULL));
    ADD_INTERRUPT_HANDLER(INT(IRQ_RX_PKT_READY));
    GPO_SET(HBC_RX_ENABLE);
}
