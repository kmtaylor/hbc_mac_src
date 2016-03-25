/* Copyright (C) 2016 Kim Taylor
 *
 * This file is part of hbc_mac.
 *
 * hbc_mac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hbc_mac is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hbc_mac.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "lcd.h"

static void sleep(void) {
    volatile int counter = 10E3;
    while (counter) counter--;
}

static void lcd_busy(void) {
    /* Have to wait 80us before checking busy flag */
    while (XIOModule_IoReadByte(&io_mod, HEX(LCD_CMD_ADDR)) & 0x80) {}
}

void lcd_init(void) {
    /* Function set */
    XIOModule_IoWriteByte(&io_mod, HEX(LCD_CMD_ADDR), 0x38);
    sleep();
    /* Function set */
    XIOModule_IoWriteByte(&io_mod, HEX(LCD_CMD_ADDR), 0x38);
    sleep();
    /* Display on, Cursor on */
    XIOModule_IoWriteByte(&io_mod, HEX(LCD_CMD_ADDR), 0x0c);
    sleep();
    /* Clear display */
    XIOModule_IoWriteByte(&io_mod, HEX(LCD_CMD_ADDR), 0x01);
    sleep();
    /* Entry mode set */
    XIOModule_IoWriteByte(&io_mod, HEX(LCD_CMD_ADDR), 0x06);
    sleep();
}

void lcd_cr(int line) {
    lcd_busy();
    if (line == 0) XIOModule_IoWriteByte(&io_mod, HEX(LCD_CMD_ADDR), 0x80);
    if (line == 1) XIOModule_IoWriteByte(&io_mod, HEX(LCD_CMD_ADDR), 0xC0);
}

void lcd_clear(void) {
    lcd_busy();
    XIOModule_IoWriteByte(&io_mod, HEX(LCD_CMD_ADDR), 0x01);
}

void lcd_clrln(int line) {
    int i;

    lcd_cr(line);

    for (i = 0; i < LCD_COLUMNS ; i++) {
	lcd_busy();
	XIOModule_IoWriteByte(&io_mod, HEX(LCD_DATA_ADDR), ' ');
    }
}

void lcd_printf(int line, const char *string) {
    int i;

    if (line >= LCD_ROWS) return;

    lcd_cr(line);

    for (i = 0; i < LCD_COLUMNS; i++) {
	if (!string[i]) return;
	lcd_busy();
	XIOModule_IoWriteByte(&io_mod, HEX(LCD_DATA_ADDR), string[i]);
    }
}

void u32_to_hex(uint32_t val, char *chars) {
    int i;
    uint8_t tmp_val;
    for (i = 0; i < 8; i++) {
	tmp_val = (val >> (i * 4)) & 0xf;
	chars[7-i] = (tmp_val < 0xa) ? (tmp_val + 0x30) : (tmp_val + 0x37);
    }
}
