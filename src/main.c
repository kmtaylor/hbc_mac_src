/* Bring up:
 * mem_if/ddr
 * data spi
 * hbc_tx
 * hbc_rx
 */

#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "lcd.h"
#include "fifo.h"
#include "usb.h"
#include "spi.h"
#include "spi_protocol.h"
#include "mem.h"
#include "build_tx.h"
#include "scrambler.h"
#include "interrupt.h"
#include "extract_rx.h"

enum ctrl_state {
    CTRL_STATE_CMD,
    CTRL_STATE_REPLY,
};

static void ctrl_cmd(uint8_t cmd) {
    static enum ctrl_state state;
    static uint8_t bytes;
    static uint32_t data;
    
    if (cmd & 0x80) state = CTRL_STATE_CMD;

    switch (state) {
	case CTRL_STATE_CMD:
	    switch (cmd) {
		case CTRL_CMD_READ_SCRAMBLER:
		    bytes = 4;
		    data = scrambler_read();
		    state = CTRL_STATE_REPLY;
		    break;
		case CTRL_CMD_READ_MEM:
		    bytes = 4;
		    mem_write(0x12345678);
		    data = mem_read();
		    state = CTRL_STATE_REPLY;
		    break;
	    }
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
    //int i;
    //uint32_t switch_val;
    XIOModule io_mod;

    XIOModule_Initialize(&io_mod, XPAR_IOMODULE_0_DEVICE_ID);

    fifo_init(&io_mod);
    mem_init(&io_mod);
    scrambler_init(&io_mod);
    hbc_spi_init(&io_mod, ctrl_cmd);
    //rx_init(&io_mod);

    setup_interrupts(&io_mod);
    //ADD_INTERRUPT(INT(IRQ_RX_FIFO_FULL));
    //ADD_INTERRUPT(INT(IRQ_RX_PKT_READY));
    ADD_INTERRUPT(INT(IRQ_HBC_CTRL_SPI));
    enable_interrupts();

    while (1) {
    }

#if 0
    plcp_header_t header_info = {
        .pilot_info = pilot_none,
        .burst_mode = 0,
        .scrambler_seed = 0,
        .PDSU_length = 255
    };

    for (i = 0; i < 64; i++) {
	mem_write(i);
    }

    while (1) {
	int_pause(1);

	switch_val = XIOModule_DiscreteRead(&io_mod, 2);

	if (switch_val & (1 << 7)) header_info.use_ri = 1;
	else header_info.use_ri = 0;

	header_info.data_rate = switch_val & 0x3;

	mem_set_rd_p(0);

#if 1
	if (header_info.use_ri) {
	    if (header_info.data_rate == 0) lcd_printf(0, "Rate: 64CPB, RI");
	    if (header_info.data_rate == 1) lcd_printf(0, "Rate: 32CPB, RI");
	    if (header_info.data_rate == 2) lcd_printf(0, "Rate: 16CPB, RI");
	    if (header_info.data_rate == 3) lcd_printf(0, "Rate: 08CPB, RI");
	} else {
	    if (header_info.data_rate == 0) lcd_printf(0, "Rate: 64CPB, SFD");
	    if (header_info.data_rate == 1) lcd_printf(0, "Rate: 32CPB, SFD");
	    if (header_info.data_rate == 2) lcd_printf(0, "Rate: 16CPB, SFD");
	    if (header_info.data_rate == 3) lcd_printf(0, "Rate: 08CPB, SFD");
	}
#endif

	disable_interrupts();
	build_tx_plcp_header(&header_info);
	build_tx_payload(&header_info);
	enable_interrupts();
    }

    return 0;
#endif
}
