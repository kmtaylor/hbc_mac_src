#define FIFO_STATUS_GPIO 1
#define FIFO_TRIGGER_GPIO 1

#define FIFO_TRIGGER		    (1 << 0)
#define FIFO_FLUSH		    (1 << 5)

#define FIFO_FULL(status)	    (status & (1 << INT(IRQ_FIFO_FULL)))
#define FIFO_ALMOST_FULL(status)    (status & (1 << INT(IRQ_FIFO_ALMOST_FULL)))
#define FIFO_OVERFLOW(status)	    (status & (1 << INT(IRQ_FIFO_OVERFLOW)))
#define FIFO_EMPTY(status)	    (status & (1 << INT(IRQ_FIFO_EMPTY)))
#define FIFO_UNDERFLOW(status)	    (status & (1 << INT(IRQ_FIFO_UNDERFLOW)))

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
