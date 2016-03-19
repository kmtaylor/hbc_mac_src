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

#define LCD_ROWS 2
#define LCD_COLUMNS 16

extern void lcd_init(void);

extern void lcd_cr(int line);
extern void lcd_clear(void);
extern void lcd_clrln(int line);

extern void lcd_printf(int line, const char *string);
extern void u32_to_hex(uint32_t val, char *chars);

#define PRINT_NUM(line, msg, val) {					    \
	memcpy(lcd_buf[line], msg, sizeof(msg) - 1);			    \
	u32_to_hex(val, &lcd_buf[line][8]);				    \
	lcd_printf(line, lcd_buf[line]);}

