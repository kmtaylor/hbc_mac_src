#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "flash.h"

void flash_init(void) {
    GPO_SET(FLASH_SS);
}

uint8_t flash_transfer(uint8_t data) {
    XIOModule_IoWriteByte(&io_mod, HEX(FLASH_ADDR), data);
    return XIOModule_IoReadByte(&io_mod, HEX(FLASH_ADDR));
}

void flash_start(void) {
    GPO_CLEAR(FLASH_SS);
}

void flash_end(void) {
    GPO_SET(FLASH_SS);
}

uint32_t flash_get_id(void) {
    uint32_t data = 0;
    flash_start();
    flash_transfer(FLASH_READ_ID);
    data |= flash_transfer(0x00);
    data <<= 8;
    data |= flash_transfer(0x00);
    data <<= 8;
    data |= flash_transfer(0x00);
    data <<= 8;
    data |= flash_transfer(0x00);
    flash_end();
    return data;
}
