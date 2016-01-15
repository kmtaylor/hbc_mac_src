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
    static int rd_index, wr_index;

    if (data_status.reading_mem) {
	rd_data |= (hbc_ctrl_read() << (rd_index * 8));
	rd_index++;

	data_req_next();

	if (rd_index == 4) {
	    mem_set_wr_p(data_status.read_addr);
	    mem_write(rd_data);
	    data_status.read_addr += rd_index;
	    rd_index = 0;
	    rd_data = 0;
	}
    }

    if (data_status.reading_arg) {
	if (data_status.arg_callback)
	    data_status.arg_callback(hbc_data_read());

	data_req_next();
    }
    
    if (data_status.writing) {
	if (wr_index == 0) {
	    mem_set_rd_p(data_status.write_addr);
	    wr_data = mem_read();
	}
	    
	XIOModule_IoWriteByte(&io_mod, HEX(SPI_DATA_ADDR), 
			(wr_data >> wr_index * 8));

	data_req_next();

	if (wr_index == 4) {
	    data_status.write_addr += wr_index;
	    wr_index = 0;
	}
    }
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
    data_req_next();
}

void hbc_data_read_to_mem_enable(int enable) {
    data_status.reading_mem = enable;
}

void hbc_data_write_from_mem_enable(int enable) {
    data_status.writing = enable;
    /* Load up first byte */
    hbc_data_irq();
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

void hbc_data_mem_read_addr_helper(void) {
    data_status.write_addr = 0;
    hbc_data_arg_enable(rd_addr_load, 1);
}

uint32_t hbc_test(void) {
    return data_status.write_addr;
}
