#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "spi.h"
#include "interrupt.h"

static XIOModule *spi_io_mod;

static void hbc_ctrl_irq(void) {
    static uint8_t i;
    hbc_ctrl_write(hbc_ctrl_read());
    i++;
    if (i % 2) XIOModule_DiscreteClear(spi_io_mod, 1, (1 << 7));
    else XIOModule_DiscreteSet(spi_io_mod, 1, (1 << 7));
}

DECLARE_HANDLER(INT(IRQ_HBC_CTRL_SPI), hbc_ctrl_irq);

void hbc_spi_init(XIOModule *io_mod) {
    spi_io_mod = io_mod;
    ADD_INTERRUPT_HANDLER(INT(IRQ_HBC_CTRL_SPI));
}

void hbc_ctrl_write(uint8_t data) {
    XIOModule_IoWriteByte(spi_io_mod, HEX(SPI_CTRL_ADDR), data);
}

uint8_t hbc_ctrl_read(void) {
    return XIOModule_IoReadByte(spi_io_mod, HEX(SPI_CTRL_ADDR));
}
