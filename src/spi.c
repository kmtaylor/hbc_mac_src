#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "spi.h"
#include "interrupt.h"

static void (*irq)(uint8_t cmd);

static void hbc_ctrl_irq(void) {
    irq(hbc_ctrl_read());
}

DECLARE_HANDLER(INT(IRQ_HBC_CTRL_SPI), hbc_ctrl_irq);

void hbc_spi_init(void (*fn)(uint8_t)) {
    irq = fn;
    ADD_INTERRUPT_HANDLER(INT(IRQ_HBC_CTRL_SPI));
    GPO_CLEAR(HBC_DATA_INT);
    GPO_OUT(HBC_DATA_INT);
}

void hbc_ctrl_write(uint8_t status, uint8_t data) {
    XIOModule_IoWriteHalfword(&io_mod, HEX(SPI_CTRL_ADDR),
		    ((status & 0xff) << 8) | (data & 0xff));
}

uint8_t hbc_ctrl_read(void) {
    return XIOModule_IoReadByte(&io_mod, HEX(SPI_CTRL_ADDR));
}

void hbc_data_write(uint8_t data) {
    /* Load up SPI data slave */
    XIOModule_IoWriteByte(&io_mod, HEX(SPI_DATA_ADDR), data);
    /* Interrupt PSOC */
    GPO_IN(HBC_DATA_INT);
    GPO_OUT(HBC_DATA_INT);
}
