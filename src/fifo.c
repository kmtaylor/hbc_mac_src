#include <stdint.h>
#include <xiomodule.h>

#include "fifo.h"
#include "interrupt.h"
#include "lcd.h"

static XIOModule *fifo_io_mod;
static int triggered;
int fifo_writing;

static inline uint32_t check_status(void) {
    return XIOModule_DiscreteRead(fifo_io_mod, FIFO_STATUS_GPIO);
}

void fifo_trigger(void) {
    if (triggered) return;
    XIOModule_DiscreteSet(fifo_io_mod, FIFO_TRIGGER_GPIO, FIFO_TRIGGER);
    XIOModule_DiscreteClear(fifo_io_mod, FIFO_TRIGGER_GPIO, FIFO_TRIGGER);
    triggered = 1;
}

void fifo_flush(void) {
    XIOModule_DiscreteSet(fifo_io_mod, FIFO_TRIGGER_GPIO, FIFO_FLUSH);
    XIOModule_DiscreteClear(fifo_io_mod, FIFO_TRIGGER_GPIO, FIFO_FLUSH);
}

void fifo_reset(void) {
    triggered = 0;
}

void fifo_write_size(uint8_t size) {
    XIOModule_IoWriteByte(fifo_io_mod, FIFO_MASK_ADDR, size);
}

#if 0
static void fifo_irq_empty(void) {
    if (fifo_writing) {
	lcd_printf(0, "FIFO empty (bad)");
        int_freeze();
    }
}

static void fifo_irq_overflow(void) {
    lcd_printf(0, "FIFO overflow");
    int_freeze();
}

DECLARE_HANDLER(IRQ_FIFO_EMPTY, fifo_irq_empty);
DECLARE_HANDLER(IRQ_FIFO_OVERFLOW, fifo_irq_overflow);
#endif

void fifo_init(XIOModule *io_mod) {
    fifo_io_mod = io_mod;
//    ADD_INTERRUPT_HANDLER(IRQ_FIFO_EMPTY);
//    ADD_INTERRUPT_HANDLER(IRQ_FIFO_OVERFLOW);
}

void fifo_write(uint32_t data) {
    /* Block until write is available */
    while (FIFO_FULL(check_status()));
    XIOModule_IoWriteWord(fifo_io_mod, FIFO_ADDR, data);
}

void fifo_modulate(uint32_t data) {
    /* Block until write is available */
    while (FIFO_ALMOST_FULL(check_status())) fifo_trigger();
    XIOModule_IoWriteWord(fifo_io_mod, MODULATOR_ADDR, data);
}

void fifo_modulate_sf(uint8_t data) {
    XIOModule_IoWriteByte(fifo_io_mod, MODULATOR_SF_ADDR, data);
}

uint32_t fifo_read(void) {
    return XIOModule_IoReadWord(fifo_io_mod, FIFO_ADDR);
}
