#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "interrupt.h"
#include "fifo.h"
#include "lcd.h"
#include "extract_rx.h"
#include "scrambler.h"
#include "build_tx.h"

extern char lcd_buf[LCD_ROWS][LCD_COLUMNS];

static XIOModule *rx_io_mod;

static inline uint32_t check_status(void) {
    return XIOModule_DiscreteRead(rx_io_mod, RX_FIFO_GPIO);
}

//uint32_t rx_read(void) {
    //
//}

static enum { packet_init, packet_reading } rx_state = packet_init;

static void rx_data_int_func(void) {
    static uint32_t sum;
    static uint32_t val;

    if (rx_state == packet_init) {
	scrambler_reseed(fifo_read() & (1 << HDR_SS_SHIFT) ? 1 : 0);
	rx_state = packet_reading;
	sum = 0;
    }

    while (RX_DATA_READY(check_status())) {
	val = fifo_read();
	sum += val ^ scrambler_read();
    }

    PRINT_NUM(1, "ChkSum:", sum);
}

static void rx_ready_int_func(void) {
    if (RX_DATA_READY(check_status())) {
	/* If there is any data left, read out the last bytes */
	rx_data_int_func();
    }
    rx_state = packet_init;
    XIOModule_DiscreteSet(rx_io_mod, RX_FIFO_GPIO, RX_PKT_ACK);
    XIOModule_DiscreteClear(rx_io_mod, RX_FIFO_GPIO, RX_PKT_ACK);
    lcd_printf(0, "Packet Complete");
}

#define RX_FIFO_FULL_INT_NO INT(IRQ_RX_FIFO_FULL)
#define RX_READY_INT_NO INT(IRQ_RX_PKT_READY)
DECLARE_HANDLER(RX_FIFO_FULL_INT_NO, rx_data_int_func);
DECLARE_HANDLER(RX_READY_INT_NO, rx_ready_int_func);

void rx_init(XIOModule *io_mod) {
    rx_io_mod = io_mod;
    ADD_INTERRUPT_HANDLER(RX_FIFO_FULL_INT_NO);
    ADD_INTERRUPT_HANDLER(RX_READY_INT_NO);
}
