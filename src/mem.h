#define MEM_RD_WR_ADDR 0x04
#define MEM_FLAGS_ADDR 0x05
#define MEM_RD_P_ADDR  0x08
#define MEM_WR_P_ADDR  0x0C

#define MEM_READ_DOWN  0x02
#define MEM_WRITE_DOWN 0x01

#define MEM_STATUS_GPIO	    1
#define MEM_PHY_INIT	    0x100

#define MEM_FIFO_FULL(status) (status & (1 << 9))

extern void mem_init(XIOModule *io_mod);

extern void mem_set_wr_p(uint32_t data);
extern void mem_set_rd_p(uint32_t data);
extern uint32_t mem_get_wr_p(void);
extern uint32_t mem_get_rd_p(void);

void mem_set_flags(uint8_t flags);
uint8_t mem_get_flags(void);

extern void mem_write(uint32_t data);
extern uint32_t mem_read(void);
