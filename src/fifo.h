#define FIFO_ADDR 0x00
#define FIFO_MASK_ADDR 0x01
#define FIFO_STATUS_GPIO 1
#define FIFO_TRIGGER_GPIO 1
#define MODULATOR_ADDR 0x18
#define MODULATOR_SF_ADDR 0x19

#define FIFO_TRIGGER		    (1 << 0)
#define FIFO_FLUSH		    (1 << 5)

#define FIFO_FULL(status)	    (status & (1 << 1))
#define FIFO_ALMOST_FULL(status)    (status & (1 << 2))
#define FIFO_OVERFLOW(status)	    (status & (1 << 3))
#define FIFO_EMPTY(status)	    (status & (1 << 4))
#define FIFO_UNDERFLOW(status)	    (status & (1 << 5))

extern int fifo_writing;
extern void fifo_init(XIOModule *io_mod);

extern void fifo_write(uint32_t data);
extern void fifo_write_size(uint8_t size);
extern uint32_t fifo_read(void);

extern void fifo_trigger(void);
extern void fifo_flush(void);
extern void fifo_reset(void);

extern void fifo_modulate_sf(uint8_t data);
extern void fifo_modulate(uint32_t data);
