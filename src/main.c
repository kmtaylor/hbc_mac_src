#include <stdint.h>
#include <string.h>
#include <xiomodule.h>

#include "lcd.h"
#include "fifo.h"
#include "mem.h"
#include "build_tx.h"
#include "interrupt.h"

char lcd_buf[LCD_ROWS][LCD_COLUMNS];
volatile int do_pause;

static void pause(void) {
    while (do_pause) {} 
}

int main() {
    XIOModule io_mod;

    XIOModule_Initialize(&io_mod, XPAR_IOMODULE_0_DEVICE_ID);

    setup_interrupts(&io_mod);
    enable_interrupts();

    memset(lcd_buf, ' ', LCD_COLUMNS * 2);
    fifo_init(&io_mod);
    lcd_init(&io_mod);
    mem_init(&io_mod);

    int i = 0;
    uint32_t tmp, error;
    error = 0;

#if 1
while (1) {
    for (i = 0; i < 0x02000000; i += 4) {
	mem_write(i);
	if ((i & 0xfffff) == 0x00000) {
	    PRINT_NUM(0, "WriteMB:", i);
	}
    }

    for (i = 0; i < 0x02000000; i += 4) {
	mem_set_rd_p(i);
	tmp = mem_read();
	if (tmp != i) error++;
	if ((i & 0xfffff) == 0x00000) {
	    PRINT_NUM(0, "Read MB:", i);
	}
    }

    PRINT_NUM(1, "ERR:", error);
}
#endif

    while (1) {
	PRINT_NUM(1, "PREAMBLE", i);
	build_tx_preamble();
	fifo_trigger();
	do_pause = 1;
	pause();
	i++;
    }

    return 0;
}
