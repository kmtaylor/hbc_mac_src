#include <stdint.h>
#include <xiomodule.h>

#include "lcd.h"

static XIOModule *lcd_io_mod;

static void sleep(void) {
    volatile int counter = 10E3;
    while (counter) counter--;
}

static void lcd_busy(void) {
    /* Have to wait 80us before checking busy flag */
    while (XIOModule_IoReadByte(lcd_io_mod, LCD_CMD_ADDR) & 0x80) {}
}

void lcd_init(XIOModule *io_mod) {
    lcd_io_mod = io_mod;
    /* Function set */
    XIOModule_IoWriteByte(lcd_io_mod, LCD_CMD_ADDR, 0x38);
    sleep();
    /* Function set */
    XIOModule_IoWriteByte(lcd_io_mod, LCD_CMD_ADDR, 0x38);
    sleep();
    /* Display on, Cursor on */
    XIOModule_IoWriteByte(lcd_io_mod, LCD_CMD_ADDR, 0x0c);
    sleep();
    /* Clear display */
    XIOModule_IoWriteByte(lcd_io_mod, LCD_CMD_ADDR, 0x01);
    sleep();
    /* Entry mode set */
    XIOModule_IoWriteByte(lcd_io_mod, LCD_CMD_ADDR, 0x06);
    sleep();
}

void lcd_cr(int line) {
    lcd_busy();
    if (line == 0) XIOModule_IoWriteByte(lcd_io_mod, LCD_CMD_ADDR, 0x80);
    if (line == 1) XIOModule_IoWriteByte(lcd_io_mod, LCD_CMD_ADDR, 0xC0);
}

void lcd_clear(void) {
    lcd_busy();
    XIOModule_IoWriteByte(lcd_io_mod, LCD_CMD_ADDR, 0x01);
}

void lcd_clrln(int line) {
    int i;

    lcd_cr(line);

    for (i = 0; i < LCD_COLUMNS ; i++) {
	lcd_busy();
	XIOModule_IoWriteByte(lcd_io_mod, LCD_DATA_ADDR, ' ');
    }
}

void lcd_printf(int line, const char *string) {
    int i;

    if (line >= LCD_ROWS) return;

    lcd_cr(line);

    for (i = 0; i < LCD_COLUMNS; i++) {
	if (!string[i]) return;
	lcd_busy();
	XIOModule_IoWriteByte(lcd_io_mod, LCD_DATA_ADDR, string[i]);
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
