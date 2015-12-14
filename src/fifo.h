#define FIFO_STATUS_GPIO	    INT(HBC_GPIO)
#define FIFO_TRIGGER_GPIO	    INT(HBC_GPIO)

#define FIFO_TRIGGER		    (1 << INT(GPO_HBC_TX_TRIGGER))
#define FIFO_FLUSH		    (1 << INT(GPO_HBC_TX_FLUSH))

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
