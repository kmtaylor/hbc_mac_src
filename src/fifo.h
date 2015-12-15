extern int fifo_writing;

extern void fifo_init(void);

extern void fifo_write(uint32_t data);
extern void fifo_write_size(uint8_t size);
extern uint32_t fifo_read(void);

extern void fifo_trigger(void);
extern void fifo_flush(void);
extern void fifo_reset(void);

extern void fifo_modulate_sf(uint8_t data);
extern void fifo_modulate(uint32_t data);
