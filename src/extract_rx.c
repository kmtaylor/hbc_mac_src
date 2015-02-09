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

uint32_t rx_read(void) {
    //
}

static void rx_int_func(void) {
    scrambler_reseed(fifo_read() & (1 << HDR_SS_SHIFT) ? 1 : 0);
    uint32_t sum = 0;
    while (RX_FIFO_DATA_READY(check_status())) {
	sum += fifo_read() ^ scrambler_read();
    }
    PRINT_NUM(1, "ChkSum:", sum);
}

#define INT_NO INT(IRQ_RX_DATA_READY)
DECLARE_HANDLER(INT_NO, rx_int_func);

void rx_init(XIOModule *io_mod) {
    rx_io_mod = io_mod;
    ADD_INTERRUPT_HANDLER(INT_NO);
}
