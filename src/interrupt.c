#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "interrupt.h"
#include "lcd.h"
#include "fifo.h"

extern char lcd_buf[LCD_ROWS][LCD_COLUMNS];

static volatile int do_pause;

static int_handler_t *int_handler_list = NULL;
static XIOModule *int_io_mod;
static uint32_t interrupt_mask;

#if int_dbg_sleep
static void sleep(void) {
    volatile int counter = 10E6;
    while (counter) counter--;
}
#endif

#if int_dbg_freeze
void _int_freeze(void) {
    while (1);
}
#endif

#if int_dbg
void _int_pause(int init_pause) {
    if (init_pause) do_pause = 1;
    while (do_pause) {}
}

void print_interrupt_info(XIOModule *io_mod, irq_line_t irq_line) {
    /* Debug IRQ */
#if int_dbg_led
    uint32_t dbg_led;
    dbg_led = XIOModule_DiscreteRead(io_mod, 1);
    //dbg_led >>= 8;
    XIOModule_DiscreteWrite(io_mod, 2, dbg_led); 
#endif

    lcd_clrln(0);

    switch (irq_line) {
	case INT(IRQ_BUTTON_2):
	    PRINT_NUM(0, "DBG Int:", XIOModule_DiscreteRead(io_mod, 1));
	    //PRINT_NUM(1, "Data:", fifo_read());
	    do_pause = 0;
	    break;
	case INT(IRQ_FIFO_FULL): 
	    lcd_printf(0, "FIFO full");
	    break;
	case INT(IRQ_CLOCK_LOSS):
	    lcd_printf(0, "Lost clock lock");
	    int_freeze();
	    break;
	case INT(IRQ_RAM_INIT):
	    lcd_printf(0, "RAM not ready");
	    break;
	case INT(IRQ_RAM_FIFO_FULL):
	    lcd_printf(0, "RAM FIFO full");
	    break;
	case INT(IRQ_USB_INT):
	    lcd_printf(0, "USB IRQ");
	    break;
	case INT(IRQ_USB_FULL):
	    lcd_printf(0, "USB Full");
	    break;
	case INT(IRQ_USB_EN):
	    lcd_printf(0, "USB Enabled");
	    break;
	case INT(IRQ_USB_EMPTY):
	    lcd_printf(0, "USB Empty");
	    break;
	case INT(IRQ_RX_DATA_READY):
	    PRINT_NUM(1, "Data:", fifo_read());
	    break;
	case INT(IRQ_BUTTON):
	    PRINT_NUM(1, "Data2:", fifo_read());
	    break;
	default :
	    break;
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

#if int_dbg
    print_interrupt_info(io_mod, irq_line);
#endif

    /* Chain other interrupt handlers */
    while (int_handler) {
	if (int_handler->irq_line == irq_line) int_handler->func();
	int_handler = int_handler->next;
    }
}

void setup_interrupts(XIOModule *io_mod) {
    int_io_mod = io_mod;
    microblaze_register_handler(int_handler, io_mod);
}

void enable_disable_interrupt(irq_line_t int_no, int enable) {
    if (enable) interrupt_mask |= (1 << (int_no + 16));
    else interrupt_mask &= ~(1 << (int_no + 16));
    XIOModule_EnableIntr(int_io_mod->BaseAddress, interrupt_mask);
}

void enable_interrupts(void) {
    microblaze_enable_interrupts();
}

void disable_interrupts(void) {
    microblaze_disable_interrupts();
}

void add_int_handler(int_handler_t *new_handler) {
    int_handler_t *last_handler = int_handler_list;
    new_handler->next = NULL;

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
