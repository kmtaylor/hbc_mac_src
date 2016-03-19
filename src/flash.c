/* Copyright (C) 2016 Kim Taylor
 *
 * This file is part of hbc_mac.
 *
 * hbc_mac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hbc_mac.  If not, see <http://www.gnu.org/licenses/>.
 */

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

void flash_bulk_erase(void) {
    flash_write_enable();
    flash_start();
    flash_transfer(FLASH_BULK_ERASE);
    flash_end();
    while (flash_read_status() & FLASH_STATUS_WIP) {}
}

void flash_sector_erase(int sector, int num) {
    while (num--) {
	flash_write_enable();
	flash_start();
	flash_transfer(FLASH_SECTOR_ERASE);
	flash_transfer(sector++);
	flash_transfer(0);
	flash_transfer(0);
	flash_end();
	while (flash_read_status() & FLASH_STATUS_WIP) {}
    }
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

void flash_read(uint32_t mem_addr, uint32_t size, uint32_t flash_addr) {
    uint32_t i;
    uint32_t data;

    mem_set_wr_p(mem_addr);

    flash_start();
    flash_transfer(FLASH_READ_DATA);
    flash_transfer(flash_addr >> 16);
    flash_transfer(flash_addr >> 8);
    flash_transfer(flash_addr);

    for (i = 0; i < size/4; i++) {
	data = flash_transfer(0x00);
	data |= flash_transfer(0x00) << 8;
	data |= flash_transfer(0x00) << 16;
	data |= flash_transfer(0x00) << 24;
	mem_write(data);
    }
    
    flash_end();
}

int flash_verify(uint32_t mem_addr, uint32_t size, uint32_t flash_addr) {
    uint32_t i;
    uint32_t data;
    int retval = 0;

    mem_set_rd_p(mem_addr);

    flash_start();
    flash_transfer(FLASH_READ_DATA);
    flash_transfer(flash_addr >> 16);
    flash_transfer(flash_addr >> 8);
    flash_transfer(flash_addr);

    for (i = 0; i < size/4; i++) {
	data = flash_transfer(0x00);
	data |= flash_transfer(0x00) << 8;
	data |= flash_transfer(0x00) << 16;
	data |= flash_transfer(0x00) << 24;
	if (data != mem_read()) retval = -1;
    }
    
    flash_end();

    return retval;
}

void flash_write(uint32_t mem_addr, uint32_t size, uint32_t flash_addr) {
    uint32_t i, page, pages, data;

    /* Issue bulk erase */
    flash_sector_erase(flash_mem_to_sector(flash_addr), 
		    flash_num_sectors(size));

    pages = flash_num_pages(size);
    page = flash_mem_to_page(flash_addr);

    mem_set_rd_p(mem_addr);

    /* Programme pages */
    while (pages--) {
	flash_write_enable();

	flash_start();
	flash_transfer(FLASH_PAGE_PROGRAM);
	flash_transfer(page >> 8);
	flash_transfer(page++);
	flash_transfer(0);

	for (i = 0; i < PAGE_SIZE/4; i++) {
	    data = mem_read();
	    flash_transfer(data);
	    flash_transfer(data >> 8);
	    flash_transfer(data >> 16);
	    flash_transfer(data >> 24);
	}
	flash_end();

	while (flash_read_status() & FLASH_STATUS_WIP) {}
    }
}
