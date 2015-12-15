#include "interrupt.h"

extern void usb_init(void);

extern void usb_write(uint32_t data);
extern uint32_t usb_read(void);

extern void usb_trigger(void);
