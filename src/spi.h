extern void hbc_spi_init(void (*fn)(uint8_t));

extern void hbc_ctrl_write(uint8_t status, uint8_t data);
extern uint8_t hbc_ctrl_read(void);

extern void hbc_data_write(uint8_t data);
extern uint8_t hbc_data_read(void);

extern void hbc_data_arg_enable(void (*callback)(uint8_t arg), int enable);
extern void hbc_data_read_to_mem_enable(int enable);
extern void hbc_data_write_from_mem_enable(int enable);

extern void hbc_data_mem_read_addr_helper(void);
extern void hbc_data_mem_write_addr_helper(void);
