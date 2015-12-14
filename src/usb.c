#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "usb.h"
#include "mem.h"
#include "interrupt.h"

static XIOModule *usb_io_mod;
static uint32_t usb_status;

static void check_status(void) {
    usb_status = XIOModule_DiscreteRead(usb_io_mod, USB_STATUS_GPIO);
}

void usb_write(uint32_t data) {
    check_status();
    if (!IRQ_STATUS(IRQ_USB_FULL, usb_status))
        XIOModule_IoWriteWord(usb_io_mod, HEX(USB_ADDR), data);
}

uint32_t usb_read(void) {
    return XIOModule_IoReadWord(usb_io_mod, HEX(USB_ADDR));
}

void usb_trigger(void) {
    XIOModule_DiscreteSet(usb_io_mod, USB_TRIGGER_GPIO, USB_TRIGGER);
    XIOModule_DiscreteClear(usb_io_mod, USB_TRIGGER_GPIO, USB_TRIGGER);
}

static void usb_int_func(void) {
    mem_write(usb_read());
}

#define INT_NO INT(IRQ_USB_INT)
DECLARE_HANDLER(INT_NO, usb_int_func);

void usb_init(XIOModule *io_mod) {
    usb_io_mod = io_mod;
    ADD_INTERRUPT_HANDLER(INT_NO);
}
