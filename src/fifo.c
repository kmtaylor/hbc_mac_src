#include <stdint.h>
#include <xiomodule.h>

#include "fifo.h"
#include "interrupt.h"
#include "lcd.h"

static XIOModule *fifo_io_mod;
static int triggered;

static inline uint32_t check_status(void) {
    return XIOModule_DiscreteRead(fifo_io_mod, FIFO_STATUS_GPIO);
}

void fifo_trigger(void) {
    if (triggered) return;
    XIOModule_DiscreteSet(fifo_io_mod, FIFO_TRIGGER_GPIO, FIFO_TRIGGER);
    XIOModule_DiscreteClear(fifo_io_mod, FIFO_TRIGGER_GPIO, FIFO_TRIGGER);
    triggered = 1;
}

void fifo_reset(void) {
    triggered = 0;
}

static int_handler_t fifo_int_handler = {
    .irq_line = IRQ_FIFO_ALMOST_FULL,
    .func = fifo_trigger,
};

void fifo_init(XIOModule *io_mod) {
    fifo_io_mod = io_mod;
    add_int_handler(&fifo_int_handler);
}

void fifo_write(uint32_t data) {
    /* Block until write is available */
    while (FIFO_FULL(check_status()));
    XIOModule_IoWriteWord(fifo_io_mod, FIFO_ADDR, data);
}

uint32_t fifo_read(void) {
    return XIOModule_IoReadWord(fifo_io_mod, FIFO_ADDR);
}
