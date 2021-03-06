/* Copyright (C) 2016 Kim Taylor
 *
 * This file is part of hbc_mac.
 *
 * hbc_mac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hbc_mac is distributed in the hope that it will be useful,
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
#include "spi.h"
#include "interrupt.h"
#include "mem.h"
#include "extract_rx.h"
#include "host_protocol.h"

static void (*irq)(uint8_t cmd);

static volatile struct {
    uint32_t load_addr;
    uint32_t dump_addr;
    uint32_t loading_mem;
    uint32_t dumping_mem;
    int dump_hbc_rx;
    int load_index;
    int dump_index;
    uint32_t ack_data;
    int ack_bytes;
    int reply_bytes;
    int reply_size;
    uint32_t reply_data;
} data_status;

void hbc_ctrl_write(uint8_t status, uint8_t data) {
    XIOModule_IoWriteHalfword(&io_mod, HEX(SPI_CTRL_ADDR),
		    ((status & 0xff) << 8) | (data & 0xff));
}

uint8_t hbc_ctrl_read(void) {
    return XIOModule_IoReadByte(&io_mod, HEX(SPI_CTRL_ADDR));
}

void hbc_data_write(uint8_t data) {
    XIOModule_IoWriteByte(&io_mod, HEX(SPI_DATA_ADDR), data);
}

uint8_t hbc_data_read(void) {
    return XIOModule_IoReadByte(&io_mod, HEX(SPI_DATA_ADDR));
}

static void data_req_next(void) {
    GPO_SET(HBC_DATA_INT);
    GPO_CLEAR(HBC_DATA_INT);
}

static void ctrl_req_next(void) {
    GPO_SET(HBC_CTRL_INT);
    GPO_CLEAR(HBC_CTRL_INT);
}

/* ------- Interrupt routines ------------------------------------------ */
/* Writes must be padded for 32 bit alignement */
static void hbc_ctrl_irq(void) {
    static uint32_t rd_data;
    uint8_t byte;

    byte = hbc_ctrl_read();

    if (data_status.loading_mem) {
	rd_data |= (byte << (data_status.load_index * 8));
	data_status.load_index++;
	data_status.loading_mem--;

	if (data_status.load_index == 4) {
	    mem_set_wr_p(data_status.load_addr);
	    mem_write(rd_data);
	    data_status.load_addr += 4;
	    data_status.load_index = 0;
	    rd_data = 0;
	}
    } else irq(byte);

    /* Acknowledge PSOC */
    ctrl_req_next();
}
DECLARE_HANDLER(INT(IRQ_HBC_CTRL_SPI), hbc_ctrl_irq);

static void hbc_data_irq(void) {
    static uint32_t wr_data;

    if (data_status.ack_bytes) {

	hbc_data_write(data_status.ack_data >>
			((REPLY_SIZE - data_status.ack_bytes) * 8));
	data_status.ack_bytes--;
	data_req_next();

    } else if (data_status.reply_bytes) {

	hbc_data_write(data_status.reply_data >> 
		    ((data_status.reply_size - data_status.reply_bytes) * 8));
	data_status.reply_bytes--;
	data_req_next();

    } else if (data_status.dumping_mem) {
	if (data_status.dump_index == 0) {
	    if (data_status.dump_hbc_rx) wr_data = rx_read();
	    else {
		mem_set_rd_p(data_status.dump_addr);
		wr_data = mem_read();
	    }
	}
	    
	hbc_data_write(wr_data >> data_status.dump_index * 8);
	data_status.dump_index++;
	data_status.dumping_mem--;

	data_req_next();

	if (data_status.dump_index == 4) {
	    data_status.dump_addr += 4;
	    data_status.dump_index = 0;
	}
    }
}
DECLARE_HANDLER(INT(IRQ_HBC_DATA_SPI), hbc_data_irq);

void hbc_spi_ack(uint32_t ack_cmd) {
    data_status.ack_bytes = REPLY_SIZE;
    data_status.ack_data = ack_cmd;
}

void hbc_spi_reply(uint32_t data, int size) {
    data_status.reply_bytes = size;
    data_status.reply_size = size;
    data_status.reply_data = data;
}

void hbc_spi_data_trigger(void) {
    hbc_data_irq();
}

void hbc_spi_dump_addr(uint32_t addr) {
    data_status.dump_addr = addr;
}

void hbc_spi_load_addr(uint32_t addr) {
    data_status.load_addr = addr;
}

void hbc_spi_dump_bytes(uint32_t bytes, int from_rx) {
    data_status.dump_index = 0;
    data_status.dump_hbc_rx = from_rx;
    data_status.dumping_mem = bytes;
}

void hbc_spi_load_bytes(uint32_t bytes, int for_tx) {
    data_status.load_index = 0;
    data_status.loading_mem = bytes;
    if (for_tx) {
	mem_set_wr_p(data_status.load_addr);
	data_status.load_addr += 4;
	mem_write(bytes);
    }
}

int hbc_spi_load_busy(void) {
    return data_status.loading_mem ? 1 : 0;
}

int hbc_spi_dump_busy(void) {
    return data_status.dumping_mem ? 1 : 0;
}

void hbc_spi_init(void (*fn)(uint8_t)) {
    irq = fn;
    ADD_INTERRUPT_HANDLER(INT(IRQ_HBC_CTRL_SPI));
    ADD_INTERRUPT_HANDLER(INT(IRQ_HBC_DATA_SPI));
    GPO_CLEAR(HBC_DATA_INT);
    GPO_OUT(HBC_DATA_INT);
    GPO_CLEAR(HBC_CTRL_INT);
    GPO_OUT(HBC_CTRL_INT);
}
