#include "interrupt.h"

#define USB_STATUS_GPIO		    INT(IRQ_GPI)
#define USB_TRIGGER_GPIO	    INT(GPP_USB_PKT_END)

#define USB_TRIGGER		    (1 << INT(GPO_USB_PKT_END))

extern void usb_init(XIOModule *io_mod);

extern void usb_write(uint32_t data);
extern uint32_t usb_read(void);

extern void usb_trigger(void);
