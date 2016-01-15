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
    CTRL_STATE_ACK,
};

XIOModule io_mod;

static volatile int send_packet;

static int rx_check_packet(void) {
    int bytes, correct = 0;
    uint32_t data, chk;

    if (!rx_packet_ready()) return -1;

    if (!rx_check_crc_ok()) goto discard_packet;
	
    bytes = rx_packet_length();
	
    if (rx_bytes_read() < bytes) goto discard_packet;

    /* Ignore packet marker and header */
    data = rx_read();
    data = rx_read();

    mem_set_rd_p(0);
    chk = mem_read();
    data = rx_read();
    while (bytes) {
	if ((chk & 0xff) == (data & 0xff)) correct++;
	chk >>= 8;
	data >>= 8;
	bytes--;
	if (bytes && ((bytes % 4) == 0)) {
	    chk = mem_read();
	    data = rx_read();
	}
    }

    rx_packet_next();
    return correct;

discard_packet:
    rx_packet_next();
    return -1;
}

/* Interrupt context - interrupts are disabled */
static void ctrl_cmd(uint8_t cmd) {
    static enum ctrl_state state;
    static uint32_t data;

    if (cmd == CTRL_CMD_START) {
	hbc_ctrl_write(CTRL_STATUS_READY, 0);
	state = CTRL_STATE_CMD;
	return;
    }
    
    switch (state) {
	case CTRL_STATE_CMD:
	    switch (cmd) {
		/* FPGA debug commands */
		case CTRL_CMD_IRQ_STATUS_READ:
		    data = XIOModule_DiscreteRead(&io_mod, INT(IRQ_GPI));
		    break;
		case CTRL_CMD_SCRAMBLER_READ:
		    data = scrambler_read();
		    break;

		/* DRAM debug commands */
		case CTRL_CMD_MEM_READ:
		    //data = mem_read();
		    //FIXME
		    data = hbc_test();
		    break;
		case CTRL_CMD_MEM_RD_ADDR:
		    hbc_data_mem_read_addr_helper();
		    break;
		case CTRL_CMD_MEM_DUMP:
		    hbc_data_write_from_mem_enable(1);
		    break;
		case CTRL_CMD_MEM_TEST:
		    data = mem_test(MEM_SIZE);
		    break;

		/* Flash debug commands */
		case CTRL_CMD_FLASH_READ:
		    flash_read(0);
		    mem_set_rd_p(0);
		    data = CTRL_CMD_FLASH_READ;
		    break;
		case CTRL_CMD_FLASH_WRITE:
		    flash_write(0, FPGA_CONFIG_SIZE);
		    data = flash_verify(0, FPGA_CONFIG_SIZE);
		    break;

		/* HBC_TX commands */
		case CTRL_CMD_HBC_TX_TRIGGER:
		    send_packet = 1;
		    break;

		/* HBC_RX commands */
		case CTRL_CMD_HBC_RX_READY:
		    data = rx_packet_ready();
		    break;
		case CTRL_CMD_HBC_RX_LENGTH:
		    data = rx_packet_length();
		    break;
		case CTRL_CMD_HBC_RX_BYTES_READ:
		    data = rx_bytes_read();
		    break;
		case CTRL_CMD_HBC_RX_CRC_OK:
		    data = rx_check_crc_ok();
		    break;
		case CTRL_CMD_HBC_RX_READ:
		    data = rx_read();
		    break;
		case CTRL_CMD_HBC_RX_NEXT:
		    rx_packet_next();
		    break;
		case CTRL_CMD_HBC_RX_CHECK:
		    data = rx_check_packet();
		    break;
	    }
	    state = CTRL_STATE_REPLY;
	    hbc_ctrl_write(CTRL_STATUS_CMD_DONE, 0);
	    break;
	case CTRL_STATE_REPLY:
	    switch (cmd) {
		case CTRL_CMD_DATA_NEXT:
		    hbc_ctrl_write(CTRL_STATUS_NEXT_DONE, data >> 24);
		    data <<= 8;
		    state = CTRL_STATE_ACK;
		    break;
	    }
	    break;
	case CTRL_STATE_ACK:
	    switch (cmd) {
		case CTRL_CMD_DATA_ACK:
		    hbc_ctrl_write(CTRL_STATUS_ACK_DONE, 0);
		    state = CTRL_STATE_REPLY;
		    break;
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
    ENABLE_INTERRUPT(INT(IRQ_HBC_DATA_SPI));
    enable_interrupts();

    GPO_SET(HBC_DATA_SWITCH);
    rx_enable();

    plcp_header_t header_info = {
	.data_rate = r_sf_8,
        .pilot_info = pilot_none,
        .burst_mode = 0,
	.use_ri = 1,
        .scrambler_seed = 0,
        .PDSU_length = 254,
    };

    while (1) {
	if (send_packet) {

	    disable_interrupts();
	    mem_set_rd_p(0);
	    build_tx_plcp_header(&header_info);
	    build_tx_payload(&header_info);
	    enable_interrupts();

	    send_packet = 0;
	}
    }

    return 0;
}
