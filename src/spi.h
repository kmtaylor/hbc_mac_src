extern void hbc_spi_init(void (*fn)(uint8_t));

extern void hbc_spi_reply(uint32_t data, int size);
extern void hbc_spi_ack(uint32_t ack_cmd);

extern void hbc_spi_dump_addr(uint32_t addr);
extern void hbc_spi_load_addr(uint32_t addr);

extern void hbc_spi_dump_bytes(uint32_t bytes, int from_rx);
extern void hbc_spi_load_bytes(uint32_t bytes, int for_tx);

extern int hbc_spi_load_busy(void);
extern int hbc_spi_dump_busy(void);

extern void hbc_spi_data_trigger(void);
