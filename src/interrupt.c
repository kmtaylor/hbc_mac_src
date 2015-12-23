#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "interrupt.h"
#include "lcd.h"
#include "fifo.h"

static int_handler_t *int_handler_list = NULL;
static uint32_t interrupt_mask;

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

#if INT_DBG
    print_interrupt_info(io_mod, irq_line);
#endif

    /* Chain other interrupt handlers */
    while (int_handler) {
	if (int_handler->irq_line == irq_line) int_handler->func();
	int_handler = int_handler->next;
    }
}

void setup_interrupts(void) {
    microblaze_register_handler(int_handler, &io_mod);
}

void enable_disable_interrupt(irq_line_t int_no, int enable) {
    if (enable) interrupt_mask |= (1 << (int_no + 16));
    else interrupt_mask &= ~(1 << (int_no + 16));
    XIOModule_EnableIntr(io_mod.BaseAddress, interrupt_mask);
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
