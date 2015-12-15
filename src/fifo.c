#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "fifo.h"
#include "interrupt.h"
#include "lcd.h"

static int triggered;
int fifo_writing;

void fifo_trigger(void) {
    if (triggered) return;
    GPO_SET(HBC_TX_TRIGGER);
    GPO_CLEAR(HBC_TX_TRIGGER);
    triggered = 1;
}

void fifo_flush(void) {
    GPO_SET(HBC_TX_FLUSH);
    GPO_CLEAR(HBC_TX_FLUSH);
}

void fifo_reset(void) {
    triggered = 0;
}

void fifo_write_size(uint8_t size) {
    XIOModule_IoWriteByte(&io_mod, HEX(FIFO_MASK_ADDR), size);
}

#if 0
static void fifo_irq_overflow(void) {
    lcd_printf(0, "FIFO overflow");
    int_freeze();
}

DECLARE_HANDLER(IRQ_FIFO_OVERFLOW, fifo_irq_overflow);
#endif

void fifo_init(void) {
//    ADD_INTERRUPT_HANDLER(IRQ_FIFO_OVERFLOW);
}

void fifo_write(uint32_t data) {
    /* Block until write is available */
    while (IRQ_FLAG_SET(IRQ_TX_FIFO_FULL));
    XIOModule_IoWriteWord(&io_mod, HEX(FIFO_ADDR), data);
}

void fifo_modulate(uint32_t data) {
    /* Block until write is available */
    while (IRQ_FLAG_SET(IRQ_TX_FIFO_ALMOST_FULL)) fifo_trigger();
    XIOModule_IoWriteWord(&io_mod, HEX(MODULATOR_ADDR), data);
}

void fifo_modulate_sf(uint8_t data) {
    XIOModule_IoWriteByte(&io_mod, HEX(MODULATOR_SF_ADDR), data);
}

uint32_t fifo_read(void) {
    return XIOModule_IoReadWord(&io_mod, HEX(RX_FIFO_ADDR));
}
