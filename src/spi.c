#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "spi.h"
#include "interrupt.h"
#include "mem.h"

static struct {
    uint32_t write_addr;
    uint32_t read_addr;
    int reading_mem:1;
    int reading_arg:1;
    int writing:1;
    int rd_index;
    int wr_index;
    void (*arg_callback)(uint8_t arg);
} data_status;

static void (*irq)(uint8_t cmd);

static void hbc_ctrl_irq(void) {
    irq(hbc_ctrl_read());
}
DECLARE_HANDLER(INT(IRQ_HBC_CTRL_SPI), hbc_ctrl_irq);

static inline void data_req_next(void) {
    GPO_IN(HBC_DATA_INT);
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
    XIOModule_IoWriteByte(&io_mod, HEX(SPI_DATA_ADDR), data);
}

uint8_t hbc_data_read(void) {
    return XIOModule_IoReadByte(&io_mod, HEX(SPI_DATA_ADDR));
}

/* Writes must be padded for 32 bit alignement */
static void hbc_data_irq(void) {
    static uint32_t rd_data, wr_data;
    int req_next = 0;

    if (data_status.reading_arg) {
	if (data_status.arg_callback)
	    data_status.arg_callback(hbc_data_read());

	req_next = 1;
    }
    
    if (data_status.reading_mem) {
	rd_data |= (hbc_data_read() << (data_status.rd_index * 8));
	data_status.rd_index++;

	req_next = 1;

	if (data_status.rd_index == 4) {
	    mem_set_wr_p(data_status.read_addr);
	    mem_write(rd_data);
	    data_status.read_addr += data_status.rd_index;
	    data_status.rd_index = 0;
	    rd_data = 0;
	}
    }

    if (data_status.writing) {
	if (data_status.wr_index == 0) {
	    mem_set_rd_p(data_status.write_addr);
	    wr_data = mem_read();
	}
	    
	hbc_data_write(wr_data >> data_status.wr_index * 8);
	data_status.wr_index++;

	req_next = 1;

	if (data_status.wr_index == 4) {
	    data_status.write_addr += data_status.wr_index;
	    data_status.wr_index = 0;
	}
    }

    if (req_next) data_req_next();
}
DECLARE_HANDLER(INT(IRQ_HBC_DATA_SPI), hbc_data_irq);

void hbc_spi_init(void (*fn)(uint8_t)) {
    irq = fn;
    ADD_INTERRUPT_HANDLER(INT(IRQ_HBC_CTRL_SPI));
    ADD_INTERRUPT_HANDLER(INT(IRQ_HBC_DATA_SPI));
    GPO_CLEAR(HBC_DATA_INT);
    GPO_OUT(HBC_DATA_INT);
}

void hbc_data_arg_enable(void (*callback)(uint8_t arg), int enable) {
    data_status.reading_arg = enable;
    data_status.arg_callback = callback;
}

void hbc_data_read_to_mem_enable(int enable) {
    data_status.reading_mem = enable;
    data_status.rd_index = 0;
}

void hbc_data_write_from_mem_enable(int enable) {
    data_status.writing = enable;
    data_status.wr_index = 0;
    /* Load up first byte */
    if (enable) hbc_data_irq();
}

static void rd_addr_load(uint8_t arg) {
    static int index;

    data_status.write_addr |= (arg << index * 8);
    index++;

    if (index == 4) {
	hbc_data_arg_enable(NULL, 0);
	index = 0;
    }
}

static void wr_addr_load(uint8_t arg) {
    static int index;

    data_status.read_addr |= (arg << index * 8);
    index++;

    if (index == 4) {
	hbc_data_arg_enable(NULL, 0);
	index = 0;
    }
}

void hbc_data_mem_read_addr_helper(void) {
    data_status.write_addr = 0;
    hbc_data_arg_enable(rd_addr_load, 1);
    data_req_next();
}

void hbc_data_mem_write_addr_helper(void) {
    data_status.read_addr = 0;
    hbc_data_arg_enable(wr_addr_load, 1);
    data_req_next();
}
