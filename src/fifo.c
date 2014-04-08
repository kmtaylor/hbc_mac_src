#include <stdint.h>
#include <xiomodule.h>

#include "fifo.h"

static XIOModule *fifo_io_mod;
static uint32_t fifo_status;

static void check_status(void) {
    fifo_status = XIOModule_DiscreteRead(fifo_io_mod, FIFO_STATUS_GPIO);
}

void fifo_init(XIOModule *io_mod) {
    fifo_io_mod = io_mod;
}

void fifo_write(uint32_t data) {
    check_status();
    if (!FIFO_FULL(fifo_status))
        XIOModule_IoWriteWord(fifo_io_mod, FIFO_ADDR, data);
}

uint32_t fifo_read(void) {
    return XIOModule_IoReadWord(fifo_io_mod, FIFO_ADDR);
}

void fifo_trigger(void) {
    XIOModule_DiscreteSet(fifo_io_mod, FIFO_TRIGGER_GPIO, FIFO_TRIGGER);
    XIOModule_DiscreteClear(fifo_io_mod, FIFO_TRIGGER_GPIO, FIFO_TRIGGER);
}
