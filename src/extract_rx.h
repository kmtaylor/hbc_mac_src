#define RX_FIFO_GPIO			INT(HBC_GPIO)

#define RX_PKT_ACK			(1 << INT(GPO_HBC_RX_PKT_ACK))

extern void rx_init(XIOModule *io_mod);

