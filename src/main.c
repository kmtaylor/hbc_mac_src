#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "lcd.h"
#include "fifo.h"
#include "usb.h"
#include "mem.h"
#include "build_tx.h"
#include "scrambler.h"
#include "interrupt.h"
#include "extract_rx.h"

char lcd_buf[LCD_ROWS][LCD_COLUMNS];

int main() {
    int i;
    uint32_t switch_val;
    XIOModule io_mod;

    XIOModule_Initialize(&io_mod, XPAR_IOMODULE_0_DEVICE_ID);

    memset(lcd_buf, ' ', LCD_COLUMNS * 2);
    lcd_init(&io_mod);
    fifo_init(&io_mod);
    usb_init(&io_mod);
    mem_init(&io_mod);
    scrambler_init(&io_mod);
    rx_init(&io_mod);

    setup_interrupts(&io_mod);
    ADD_INTERRUPT(INT(IRQ_BUTTON));
    ADD_INTERRUPT(INT(IRQ_RX_DATA_READY));
    ADD_INTERRUPT(INT(IRQ_BUTTON_2));
    enable_interrupts();
    
    plcp_header_t header_info = {
        .pilot_info = pilot_none,
        .burst_mode = 0,
        .scrambler_seed = 0,
        .PDSU_length = 32
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

	disable_interrupts();
	build_tx_plcp_header(&header_info);
	build_tx_payload(&header_info);
	enable_interrupts();
    }

    return 0;
}
