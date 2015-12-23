#define MEM_SIZE	    (1024*1024*32)
#define MEM_TX_BUF	    0
#define MEM_RX_BUF	    (MEM_SIZE/2)

#define MEM_READ_WRAP_16M   0x02
#define MEM_WRITE_WRAP_16M  0x01

extern void mem_init(void);

extern void mem_set_wr_p(uint32_t data);
extern void mem_set_rd_p(uint32_t data);
extern uint32_t mem_get_wr_p(void);
extern uint32_t mem_get_rd_p(void);

void mem_set_flags(uint8_t flags);
uint8_t mem_get_flags(void);

extern void mem_write(uint32_t data);
extern uint32_t mem_read(void);

extern uint8_t mem_test(uint32_t bytes);
