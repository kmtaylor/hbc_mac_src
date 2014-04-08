typedef enum {
    IRQ_BUTTON,				    /* 0 */
    IRQ_FIFO_FULL,			    /* 1 */
    IRQ_FIFO_ALMOST_FULL,		    /* 2 */
    IRQ_FIFO_OVERFLOW,			    /* 3 */
    IRQ_FIFO_EMPTY,			    /* 4 */
    IRQ_FIFO_ALMOST_EMPTY,		    /* 5 */
    IRQ_FIFO_UNDERFLOW,			    /* 6 */
    IRQ_CLOCK_LOSS,			    /* 7 */
    IRQ_RAM_INIT,			    /* 8 */
    IRQ_RAM_FIFO_FULL } irq_line_t;

extern void setup_interrupts(XIOModule *io_mod);
extern void enable_interrupts(void);
