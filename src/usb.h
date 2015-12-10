#include "interrupt.h"

#define USB_STATUS_GPIO 1
#define USB_TRIGGER_GPIO 1

#define USB_TRIGGER		    (1 << 1)

#define USB_FULL(status)	    (status & (1 << 11))
#define	USB_EMPTY(status)	    (status & (1 << 13))
#define USB_ENABLED(status)	    (status & (1 << 12))

extern void usb_init(XIOModule *io_mod);

extern void usb_write(uint32_t data);
extern uint32_t usb_read(void);

extern void usb_trigger(void);
