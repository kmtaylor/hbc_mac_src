#include <stdint.h>
#include <string.h>
#include <xiomodule.h>

#include "interrupt.h"
#include "lcd.h"
#include "fifo.h"

extern char lcd_buf[LCD_ROWS][LCD_COLUMNS];

static volatile int do_pause;

static int_handler_t *int_handler_list = NULL;

#if int_dbg_sleep
static void sleep(void) {
    volatile int counter = 10E6;
    while (counter) counter--;
}
#endif

#if int_dbg
void _int_pause(int init_pause) {
    if (init_pause) do_pause = 1;
    while (do_pause) {}
}

void print_interrupt_info(XIOModule *io_mod, irq_line_t irq_line) {
    /* Debug IRQ */
    uint32_t dbg_led;
    dbg_led = XIOModule_DiscreteRead(io_mod, 1);
    dbg_led >>= 8;
    XIOModule_DiscreteWrite(io_mod, 2, dbg_led); 

    lcd_clrln(0);

    switch (irq_line) {
	case IRQ_BUTTON :
	    PRINT_NUM(0, "DBG Int:", XIOModule_DiscreteRead(io_mod, 1));
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
	    break;
	case IRQ_RAM_INIT :
	    lcd_printf(0, "RAM not ready");
	    break;
	case IRQ_RAM_FIFO_FULL :
	    lcd_printf(0, "RAM FIFO full");
	    break;
	case IRQ_USB_INT :
	    lcd_printf(0, "USB IRQ");
	    break;
	case IRQ_USB_FULL :
	    lcd_printf(0, "USB Full");
	    break;
	case IRQ_USB_EN :
	    lcd_printf(0, "USB Enabled");
	    break;
	case IRQ_USB_EMPTY :
	    lcd_printf(0, "USB Empty");
	    break;
	default :
	    lcd_printf(0, "Bad Interrupt!");
    }
#if int_dbg_sleep
    sleep();
#endif
}
#endif /* int_dbg */

static void int_handler(void *io_mod_p) {
    uint32_t pending_mask;
    irq_line_t irq_line;
    int_handler_t *int_handler = int_handler_list;
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

    /* Chain other interrupt handlers */
    while (int_handler) {
	if (int_handler->irq_line == irq_line) int_handler->func();
	int_handler = int_handler->next;
    }

#if int_dbg
    print_interrupt_info(io_mod, irq_line);
#endif
}

void setup_interrupts(XIOModule *io_mod) {
    microblaze_register_handler(int_handler, io_mod);
    XIOModule_EnableIntr(io_mod->BaseAddress, 0x3fff0000);
}

void enable_interrupts(void) {
    microblaze_enable_interrupts();
}

void add_int_handler(int_handler_t *new_handler) {
    int_handler_t *last_handler = int_handler_list;

    if (!last_handler) {
	int_handler_list = new_handler;
	return;
    }

    while (last_handler) {
	if (!last_handler->next) break;
	last_handler = last_handler->next;
    }
    last_handler->next = new_handler; 
}
