#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "usb.h"
#include "mem.h"
#include "interrupt.h"

void usb_write(uint32_t data) {
    if (!IRQ_FLAG_SET(IRQ_USB_FULL))
        XIOModule_IoWriteWord(&io_mod, HEX(USB_ADDR), data);
}

uint32_t usb_read(void) {
    return XIOModule_IoReadWord(&io_mod, HEX(USB_ADDR));
}

void usb_trigger(void) {
    GPO_SET(USB_PKT_END);
    GPO_CLEAR(USB_PKT_END);
}

static void usb_int_func(void) {
    mem_write(usb_read());
}

#define INT_NO INT(IRQ_USB_INT)
DECLARE_HANDLER(INT_NO, usb_int_func);

void usb_init(void) {
    ADD_INTERRUPT_HANDLER(INT_NO);
}
