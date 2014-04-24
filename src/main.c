#include <stdint.h>
#include <string.h>
#include <xiomodule.h>

#include "lcd.h"
#include "fifo.h"
#include "usb.h"
#include "mem.h"
#include "build_tx.h"
#include "interrupt.h"

char lcd_buf[LCD_ROWS][LCD_COLUMNS];

int main() {
    uint32_t tmp;
    int i;
    XIOModule io_mod;

    XIOModule_Initialize(&io_mod, XPAR_IOMODULE_0_DEVICE_ID);

    setup_interrupts(&io_mod);
    enable_interrupts();

    memset(lcd_buf, ' ', LCD_COLUMNS * 2);
    fifo_init(&io_mod);
    usb_init(&io_mod);
    lcd_init(&io_mod);
    mem_init(&io_mod);

    while (1) {
    }

    return 0;
}
