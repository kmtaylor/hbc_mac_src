extern void hbc_spi_init(void (*fn)(uint8_t));
extern void hbc_ctrl_write(uint8_t status, uint8_t data);
extern uint8_t hbc_ctrl_read(void);
extern void hbc_data_write(uint8_t data);
