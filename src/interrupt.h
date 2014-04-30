#ifndef INTERRUPT_H
#define INTERRUPT_H

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
    IRQ_RAM_FIFO_FULL,			    /* 9 */
    IRQ_USB_INT,			    /* 10 */
    IRQ_USB_FULL,			    /* 11 */
    IRQ_USB_EN,				    /* 12 */
    IRQ_USB_EMPTY			    /* 13 */
} irq_line_t;

extern void setup_interrupts(XIOModule *io_mod);
extern void enable_interrupts(void);
extern void _int_pause(int init_pause);

typedef struct s_int_handler int_handler_t;

struct s_int_handler {
    irq_line_t irq_line;
    void (*func)(void);
    int_handler_t *next;
};

extern void add_int_handler(int_handler_t *new_handler);

#define int_dbg 1
#define int_dbg_sleep 0

#if int_dbg
#define int_pause(init_pause) _int_pause(init_pause)
#else
#define int_pause(init_pause) do {} while (0)
#endif

#endif /* INTERRUPT_H */
