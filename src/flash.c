#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "flash.h"
#include "mem.h"

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

static void flash_write_enable(void) {
    flash_start();
    flash_transfer(FLASH_WRITE_ENABLE);
    flash_end();
}

static void flash_bulk_erase(void) {
    flash_write_enable();
    flash_start();
    flash_transfer(FLASH_BULK_ERASE);
    flash_end();
    while (flash_read_status() & FLASH_STATUS_WIP) {}
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

uint8_t flash_read_status(void) {
    uint8_t data;
    flash_start();
    flash_transfer(FLASH_READ_STATUS);
    data = flash_transfer(0x00);
    flash_end();
    return data;
}

void flash_read(uint32_t mem_addr) {
    uint32_t i;
    uint32_t data;

    mem_set_wr_p(mem_addr);

    flash_start();
    flash_transfer(FLASH_READ_DATA);
    flash_transfer(0);
    flash_transfer(0);
    flash_transfer(0);

    for (i = 0; i < FLASH_SIZE/4; i++) {
	data = 0;
	data |= flash_transfer(0x00);
	data <<= 8;
	data |= flash_transfer(0x00);
	data <<= 8;
	data |= flash_transfer(0x00);
	data <<= 8;
	data |= flash_transfer(0x00);
	mem_write(data);
    }
    
    flash_end();
}

int flash_verify(uint32_t mem_addr, uint32_t size) {
    uint32_t i;
    uint32_t data;
    int retval = 0;

    mem_set_rd_p(mem_addr);

    flash_start();
    flash_transfer(FLASH_READ_DATA);
    flash_transfer(0);
    flash_transfer(0);
    flash_transfer(0);

    for (i = 0; i < size/4; i++) {
	data = 0;
	data |= flash_transfer(0x00);
	data <<= 8;
	data |= flash_transfer(0x00);
	data <<= 8;
	data |= flash_transfer(0x00);
	data <<= 8;
	data |= flash_transfer(0x00);
	if (data != mem_read()) retval = -1;
    }
    
    flash_end();

    return retval;
}

void flash_write(uint32_t mem_addr, uint32_t size) {
    uint32_t i, page, pages, data;

    /* Issue bulk erase */
    flash_bulk_erase();

    pages = size/PAGE_SIZE;
    if (size % PAGE_SIZE) pages++;

    mem_set_rd_p(mem_addr);

    /* Programme pages */
    for (page = 0; page < pages; page++) {
	flash_write_enable();

	flash_start();
	flash_transfer(FLASH_PAGE_PROGRAM);
	flash_transfer(page >> 8);
	flash_transfer(page);
	flash_transfer(0);

	for (i = 0; i < PAGE_SIZE/4; i++) {
	    data = mem_read();
	    flash_transfer(data >> 24);
	    flash_transfer(data >> 16);
	    flash_transfer(data >> 8);
	    flash_transfer(data);
	}
	flash_end();

	while (flash_read_status() & FLASH_STATUS_WIP) {}
    }
}
