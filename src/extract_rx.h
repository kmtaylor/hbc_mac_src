extern void rx_init(void);

extern int rx_packet_ready(void);
extern uint8_t rx_packet_length(void);
extern int rx_check_crc_ok(void);
extern uint32_t rx_read(void);
extern void rx_packet_next(void);
extern uint32_t rx_bytes_read(void);
extern void rx_enable(void);
extern void rx_disable(void);

extern uint32_t rx_read_addr(void);
