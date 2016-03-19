/* Copyright (C) 2016 Kim Taylor
 *
 * This file is part of hbc_mac.
 *
 * hbc_mac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hbc_mac.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "interrupt.h"
#include "lcd.h"
#include "fifo.h"

#if INT_DBG_SLEEP
static void sleep(void) {
    volatile int counter = 10E6;
    while (counter) counter--;
}
#endif

#if INT_DBG_FREEZE
void _int_freeze(void) {
    while (1);
}
#endif

#if INT_DBG
static volatile int do_pause;
extern char lcd_buf[LCD_ROWS][LCD_COLUMNS];

void _int_pause(int init_pause) {
    if (init_pause) do_pause = 1;
    while (do_pause) {}
}

void print_interrupt_info(XIOModule *io_mod, irq_line_t irq_line) {
    /* Debug IRQ */
#if INT_DBG_LED
    uint32_t dbg_led;
    dbg_led = XIOModule_DiscreteRead(io_mod, 1);
    //dbg_led >>= 8;
    XIOModule_DiscreteWrite(io_mod, 2, dbg_led); 
#endif

    lcd_clrln(0);

    switch (irq_line) {
	case INT(IRQ_BUTTON_2):
	    PRINT_NUM(0, "DBG Int:", XIOModule_DiscreteRead(io_mod, 1));
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
	case INT(IRQ_BUTTON):
	    PRINT_NUM(1, "FIFO_RD:", fifo_read());
	    break;
	default :
	    break;
    }
#if INT_DBG_SLEEP
    sleep();
#endif
}
#endif /* INT_DBG */
