extern void hbc_spi_init(void (*fn)(uint8_t));

extern void hbc_spi_reply(uint32_t data, int size);
extern void hbc_spi_reply_trigger(void);
extern void hbc_spi_ack(uint8_t ack_cmd);

extern void hbc_spi_dump_addr(uint32_t addr);
extern void hbc_spi_load_addr(uint32_t addr);

extern void hbc_spi_dump_bytes(uint32_t bytes);
extern void hbc_spi_load_bytes(uint32_t bytes);
