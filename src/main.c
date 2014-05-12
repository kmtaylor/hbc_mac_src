#include <stdint.h>
#include <string.h>
#include <xiomodule.h>

#include "lcd.h"
#include "fifo.h"
#include "usb.h"
#include "mem.h"
#include "build_tx.h"
#include "scrambler.h"
#include "interrupt.h"

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

    setup_interrupts(&io_mod);
    ADD_INTERRUPT(IRQ_BUTTON);
    enable_interrupts();
    
    plcp_header_t header_info = {
        .pilot_info = pilot_none,
        .burst_mode = 0,
        .scrambler_seed = 0,
        .PDSU_length = 255
    };

    for (i = 0; i < 64; i++) {
	mem_write(i);
    }

    i = 0;

    while (1) {
	int_pause(1);

	switch_val = XIOModule_DiscreteRead(&io_mod, 2);

	if (switch_val & (1 << 7)) header_info.use_ri = 1;
	else header_info.use_ri = 0;

	header_info.data_rate = switch_val & 0x3;

	mem_set_rd_p(0);

	disable_interrupts();
	build_tx_plcp_header(&header_info);
	build_tx_payload(&header_info);
	enable_interrupts();

	PRINT_NUM(1, "PACK:", ++i);
    }

    return 0;
}
