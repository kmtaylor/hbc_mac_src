#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "spi.h"
#include "interrupt.h"

static XIOModule *spi_io_mod;
static void (*irq)(uint8_t cmd);

static void hbc_ctrl_irq(void) {
    irq(hbc_ctrl_read());
}

DECLARE_HANDLER(INT(IRQ_HBC_CTRL_SPI), hbc_ctrl_irq);

void hbc_spi_init(XIOModule *io_mod, void (*fn)(uint8_t)) {
    spi_io_mod = io_mod;
    irq = fn;
    ADD_INTERRUPT_HANDLER(INT(IRQ_HBC_CTRL_SPI));
}

void hbc_ctrl_write(uint16_t data) {
    XIOModule_IoWriteHalfword(spi_io_mod, HEX(SPI_CTRL_ADDR), data);
}

uint8_t hbc_ctrl_read(void) {
    return XIOModule_IoReadByte(spi_io_mod, HEX(SPI_CTRL_ADDR));
}
