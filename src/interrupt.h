#ifndef INTERRUPT_H
#define INTERRUPT_H

typedef int irq_line_t;

extern void setup_interrupts(XIOModule *io_mod);
extern void enable_disable_interrupt(irq_line_t int_no, int enable);
extern void enable_interrupts(void);
extern void disable_interrupts(void);
extern void _int_pause(int init_pause);
extern void _int_freeze(void);

typedef struct s_int_handler int_handler_t;

struct s_int_handler {
    irq_line_t irq_line;
    void (*func)(void);
    int_handler_t *next;
};

extern void add_int_handler(int_handler_t *new_handler);

#define ADD_INTERRUPT_HANDLER(int_no) { \
	add_int_handler(&__##int_no##_struct);				    \
	enable_disable_interrupt(int_no, 1);}

#define ADD_INTERRUPT(int_no) enable_disable_interrupt(int_no, 1)

#define REM_INTERRUPT(int_no) enable_disable_interrupt(int_no, 0)

#define DECLARE_HANDLER(int_no, handler_func)  \
	static int_handler_t __##int_no##_struct = {			    \
	    .irq_line = int_no,						    \
	    .func = handler_func,					    \
	}

#define int_dbg 1
#define int_dbg_sleep 0
#define int_dbg_led 0
#define int_dbg_freeze 1

#if int_dbg_freeze
#define int_freeze() _int_freeze()
#else
#define int_freeze() do {} while (0)
#endif

#if int_dbg
#define int_pause(init_pause) _int_pause(init_pause)
#else
#define int_pause(init_pause) do {} while (0)
#endif

#endif /* INTERRUPT_H */
