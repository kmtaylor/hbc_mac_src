#define MEM_READ_DOWN  0x02
#define MEM_WRITE_DOWN 0x01

#define MEM_STATUS_GPIO	    INT(IRQ_GPI)
#define MEM_PHY_INIT	    0x100

extern void mem_init(XIOModule *io_mod);

extern void mem_set_wr_p(uint32_t data);
extern void mem_set_rd_p(uint32_t data);
extern uint32_t mem_get_wr_p(void);
extern uint32_t mem_get_rd_p(void);

void mem_set_flags(uint8_t flags);
uint8_t mem_get_flags(void);

extern void mem_write(uint32_t data);
extern uint32_t mem_read(void);
