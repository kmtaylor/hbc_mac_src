#include <stdint.h>
#include <string.h>
#include <xiomodule.h>

#include "interrupt.h"
#include "lcd.h"
#include "fifo.h"

extern char lcd_buf[LCD_ROWS][LCD_COLUMNS];
extern volatile int do_pause;

#define int_dbg 0
#if int_dbg
static void sleep(void) {
    volatile int counter = 10E6;
    while (counter) counter--;
}
#endif

static void int_handler(void *io_mod_p) {
    uint32_t pending_mask;
    irq_line_t irq_line;
    static int interrupt_count;
    XIOModule *io_mod = (XIOModule *) io_mod_p;

    interrupt_count++;

    /* Check which interrupt we got */
    pending_mask = XIOModule_GetIntrStatus(io_mod->BaseAddress);
    for (irq_line = 0; irq_line < 16; irq_line++) {
	if (pending_mask & (1 << (irq_line + 16))) break;
    }
    /* Acknowledge this interrupt */
    XIOModule_AckIntr(io_mod->BaseAddress, (1 << (irq_line + 16)));

    lcd_clrln(0);

#if int_dbg
    /* Debug IRQ */
    uint32_t dbg_led;
    dbg_led = XIOModule_DiscreteRead(io_mod, 1);
    dbg_led >>= 8;
    XIOModule_DiscreteWrite(io_mod, 2, dbg_led); 
#endif

    switch (irq_line) {
	case IRQ_BUTTON :
	    PRINT_NUM(0, "DBG Int:",
		    XIOModule_DiscreteRead(io_mod, FIFO_STATUS_GPIO));
	    do_pause = 0;
	    break;
	case IRQ_FIFO_FULL : 
	    lcd_printf(0, "FIFO full");
	    break;
	case IRQ_FIFO_ALMOST_FULL :
	    lcd_printf(0, "FIFO almost full");
	    break;
	case IRQ_FIFO_OVERFLOW :
	    lcd_printf(0, "FIFO overflow");
	    break;
	case IRQ_FIFO_EMPTY :
	    lcd_printf(0, "FIFO empty");
	    break;
	case IRQ_FIFO_ALMOST_EMPTY :
	    lcd_printf(0, "FIFO almost emty");
	    break;
	case IRQ_FIFO_UNDERFLOW :
	    lcd_printf(0, "FIFO underflow");
	    break;
	case IRQ_CLOCK_LOSS :
	    lcd_printf(0, "Lost clock lock");
	    do_pause = 1;
	    break;
	case IRQ_RAM_INIT :
	    lcd_printf(0, "RAM not ready");
	    do_pause = 1;
	    break;
	case IRQ_RAM_FIFO_FULL :
	    lcd_printf(0, "RAM FIFO full");
	    do_pause = 1;
	    break;
	default :
	    lcd_printf(0, "Bad Interrupt!");
    }
#if int_dbg
    sleep();
#endif
}

void setup_interrupts(XIOModule *io_mod) {
    microblaze_register_handler(int_handler, io_mod);
    XIOModule_EnableIntr(io_mod->BaseAddress, 0x3ff0000);
}

void enable_interrupts(void) {
    microblaze_enable_interrupts();
}
