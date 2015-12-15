extern void hbc_spi_init(XIOModule *io_mod, void (*fn)(uint8_t));
extern void hbc_ctrl_write(uint16_t data);
extern uint8_t hbc_ctrl_read(void);
