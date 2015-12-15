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

