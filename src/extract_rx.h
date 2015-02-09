#define RX_FIFO_GPIO 1
#define RX_FIFO_DATA_READY(status)	(status & (1 << INT(IRQ_RX_DATA_READY)))

extern void rx_init(XIOModule *io_mod);

