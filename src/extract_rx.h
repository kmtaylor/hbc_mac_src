#define RX_FIFO_GPIO 1
#define RX_FIFO_FULL(status)		(status & (1 << INT(IRQ_RX_FIFO_FULL)))
#define RX_DATA_READY(status)		(status & (1 << INT(IRQ_RX_DATA_READY)))

#define RX_PKT_ACK			(1 << 6)

extern void rx_init(XIOModule *io_mod);

