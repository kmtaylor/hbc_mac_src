/* Copyright (C) 2016 Kim Taylor
 *
 * This file is part of hbc_mac.
 *
 * hbc_mac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hbc_mac.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

typedef int irq_line_t;

extern void setup_interrupts(void);
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

#define _ADD_INTERRUPT_HANDLER(int_no) add_int_handler(&__##int_no##_struct)
#define ADD_INTERRUPT_HANDLER(int_no) _ADD_INTERRUPT_HANDLER(int_no)

#define ENABLE_INTERRUPT(int_no) enable_disable_interrupt(int_no, 1)

#define DISABLE_INTERRUPT(int_no) enable_disable_interrupt(int_no, 0)

#define _DECLARE_HANDLER(int_no, handler_func)  \
	static int_handler_t __##int_no##_struct = {			    \
	    .irq_line = int_no,						    \
	    .func = handler_func,					    \
	}
#define DECLARE_HANDLER(int_no, handler_func) \
	_DECLARE_HANDLER(int_no, handler_func)

#define IRQ_FLAG_SET(irq) \
	IRQ_STATUS(irq, XIOModule_DiscreteRead(&io_mod, INT(IRQ_GPI)))

#define INT_DBG 0
#define INT_DBG_SLEEP 0
#define INT_DBG_LED 0
#define INT_DBG_FREEZE 0

#if INT_DBG_FREEZE
#define int_freeze() _int_freeze()
#else
#define int_freeze() do {} while (0)
#endif

#if INT_DBG
#define int_pause(init_pause) _int_pause(init_pause)
#else
#define int_pause(init_pause) do {} while (0)
#endif

#endif /* INTERRUPT_H */
