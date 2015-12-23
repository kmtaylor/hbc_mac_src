#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "lcd.h"
#include "fifo.h"
#include "usb.h"
#include "spi.h"
#include "spi_protocol.h"
#include "flash.h"
#include "mem.h"
#include "build_tx.h"
#include "scrambler.h"
#include "interrupt.h"
#include "extract_rx.h"

enum ctrl_state {
    CTRL_STATE_CMD,
    CTRL_STATE_REPLY,
};

XIOModule io_mod;

static volatile int send_packet;

static void ctrl_cmd(uint8_t cmd) {
    static enum ctrl_state state;
    static uint8_t bytes;
    static uint32_t data;
    
    if (cmd & 0x80) state = CTRL_STATE_CMD;

    switch (state) {
	case CTRL_STATE_CMD:
	    hbc_ctrl_write(CTRL_STATUS_EMPTY << 8);
	    switch (cmd) {
		case CTRL_CMD_READ_SCRAMBLER:
		    bytes = 4;
		    data = scrambler_read();
		    state = CTRL_STATE_REPLY;
		    break;
		case CTRL_CMD_READ_MEM:
		    bytes = 4;
		    data = mem_read();
		    state = CTRL_STATE_REPLY;
		    break;
		case CTRL_CMD_TEST_MEM:
		    data = mem_test(4096);
		    hbc_data_write(data);
		    break;
		case CTRL_CMD_WRITE_FLASH:
		    flash_write(0, FPGA_CONFIG_SIZE);
		    data = flash_verify(0, FPGA_CONFIG_SIZE);
		    hbc_data_write(data);
		    break;
		case CTRL_CMD_READ_FLASH:
		    flash_read(0);
		    mem_set_rd_p(0);
		    hbc_data_write(CTRL_CMD_READ_FLASH);
		    break;
		case CTRL_CMD_HBC_TRIGGER:
		    send_packet = 1;
		    break;
		case CTRL_CMD_GET_IRQ:
		    bytes = 4;
		    data = XIOModule_DiscreteRead(&io_mod, INT(IRQ_GPI));
		    state = CTRL_STATE_REPLY;
		    break;
		case CTRL_CMD_RX_1:
		    bytes = 4;
		    data = rx_packet_ready();
		    state = CTRL_STATE_REPLY;
		    break;
		case CTRL_CMD_RX_2:
		    bytes = 4;
		    data = rx_packet_length();
		    state = CTRL_STATE_REPLY;
		    break;
		case CTRL_CMD_RX_3:
		    bytes = 4;
		    data = rx_check_crc_ok();
		    state = CTRL_STATE_REPLY;
		    break;
		case CTRL_CMD_RX_4:
		    bytes = 4;
		    data = rx_read();
		    state = CTRL_STATE_REPLY;
		    break;
		case CTRL_CMD_RX_5:
		    rx_packet_next();
		    break;
	    }
	    hbc_ctrl_write(CTRL_STATUS_CMD_DONE << 8);
	    break;
	case CTRL_STATE_REPLY:
	    switch (cmd) {
		case CTRL_CMD_DATA_NEXT:
		    if (bytes) {
			hbc_ctrl_write((data >> 24) | (CTRL_STATUS_OK << 8));
			data <<= 8;
			bytes--;
		    }
		    break;
		case CTRL_CMD_DATA:
		    hbc_ctrl_write(CTRL_STATUS_EMPTY << 8);
	    }
	    break;
    }
}

int main() {
    XIOModule_Initialize(&io_mod, XPAR_IOMODULE_0_DEVICE_ID);

    fifo_init();
    mem_init();
    flash_init();
    rx_init();
    hbc_spi_init(ctrl_cmd);

    /* Divide memory into RX and TX circular buffers */
    mem_set_flags(MEM_READ_WRAP_16M | MEM_WRITE_WRAP_16M);

    setup_interrupts();
    ENABLE_INTERRUPT(INT(IRQ_RX_FIFO_ALMOST_FULL));
    ENABLE_INTERRUPT(INT(IRQ_RX_PKT_READY));
    ENABLE_INTERRUPT(INT(IRQ_HBC_CTRL_SPI));
    enable_interrupts();

    plcp_header_t header_info = {
	.data_rate = r_sf_64,
        .pilot_info = pilot_none,
        .burst_mode = 0,
	.use_ri = 1,
        .scrambler_seed = 0,
        .PDSU_length = 255,
    };

    while (1) {
	if (send_packet) {
	    mem_set_rd_p(0);

	    disable_interrupts();
	    build_tx_plcp_header(&header_info);
	    build_tx_payload(&header_info);
	    enable_interrupts();

	    send_packet = 0;
	}
    }

    return 0;
}
