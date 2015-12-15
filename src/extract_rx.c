#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "interrupt.h"
#include "fifo.h"
#include "lcd.h"
#include "extract_rx.h"
#include "scrambler.h"
#include "build_tx.h"

static uint32_t sum;

uint32_t rx_read(void) {
    return sum;
}

static enum { packet_init, packet_reading } rx_state = packet_init;

static void rx_data_int_func(void) {
    static uint32_t val;

    if (rx_state == packet_init) {
	scrambler_reseed(fifo_read() & (1 << HDR_SS_SHIFT) ? 1 : 0);
	rx_state = packet_reading;
	sum = 0;
    }

    while (IRQ_FLAG_SET(IRQ_RX_DATA_READY)) {
	val = fifo_read();
	sum += val ^ scrambler_read();
    }
}

static void rx_ready_int_func(void) {
    if (IRQ_FLAG_SET(IRQ_RX_DATA_READY)) {
	/* If there is any data left, read out the last bytes */
	rx_data_int_func();
    }
    rx_state = packet_init;
    GPO_SET(HBC_RX_PKT_ACK);
    GPO_CLEAR(HBC_RX_PKT_ACK);
}

DECLARE_HANDLER(INT(IRQ_RX_FIFO_FULL), rx_data_int_func);
DECLARE_HANDLER(INT(IRQ_RX_PKT_READY), rx_ready_int_func);

void rx_init(void) {
    ADD_INTERRUPT_HANDLER(INT(IRQ_RX_FIFO_FULL));
    ADD_INTERRUPT_HANDLER(INT(IRQ_RX_PKT_READY));
}
