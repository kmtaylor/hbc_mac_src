/* Copyright (C) 2016 Kim Taylor
 *
 * This file is part of hbc_mac.
 *
 * hbc_mac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * hbc_mac is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hbc_mac.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "fifo.h"
#include "interrupt.h"
#include "lcd.h"

static int triggered;

void fifo_trigger(void) {
    if (triggered) return;
    GPO_SET(HBC_TX_TRIGGER);
    GPO_CLEAR(HBC_TX_TRIGGER);
    triggered = 1;
}

void fifo_flush(void) {
    GPO_SET(HBC_TX_FLUSH);
    GPO_CLEAR(HBC_TX_FLUSH);
}

void fifo_reset(void) {
    triggered = 0;
}

void fifo_wait(void) {
    while (!IRQ_FLAG_SET(IRQ_TX_FIFO_EMPTY)) {}
}

void fifo_write_size(uint8_t size) {
    XIOModule_IoWriteByte(&io_mod, HEX(FIFO_MASK_ADDR), size);
}

void fifo_write(uint32_t data) {
    /* Block until write is available */
    while (IRQ_FLAG_SET(IRQ_TX_FIFO_ALMOST_FULL)) fifo_trigger();
    XIOModule_IoWriteWord(&io_mod, HEX(FIFO_ADDR), data);
}

void fifo_modulate(uint32_t data) {
    /* Block until write is available */
    while (IRQ_FLAG_SET(IRQ_TX_FIFO_ALMOST_FULL)) fifo_trigger();
    XIOModule_IoWriteWord(&io_mod, HEX(MODULATOR_ADDR), data);
}

void fifo_modulate_sf(uint8_t data) {
    XIOModule_IoWriteByte(&io_mod, HEX(MODULATOR_SF_ADDR), data);
}

uint32_t fifo_read(void) {
    return XIOModule_IoReadWord(&io_mod, HEX(RX_FIFO_ADDR));
}

#if FIFO_IRQ_DEBUG
static void fifo_irq_tx_empty(void) {
}
DECLARE_HANDLER(INT(IRQ_TX_FIFO_EMPTY), fifo_irq_tx_empty);

static void fifo_irq_tx_almost_full(void) {
}
DECLARE_HANDLER(INT(IRQ_TX_FIFO_ALMOST_FULL), fifo_irq_tx_almost_full);

static void fifo_irq_tx_overflow(void) {
}
DECLARE_HANDLER(INT(IRQ_TX_FIFO_OVERFLOW), fifo_irq_tx_overflow);
#endif

void fifo_init(void) {
#if FIFO_IRQ_DEBUG
    ADD_INTERRUPT_HANDLER(INT(IRQ_TX_FIFO_EMPTY));
    ADD_INTERRUPT_HANDLER(INT(IRQ_TX_FIFO_ALMOST_FULL));
    ADD_INTERRUPT_HANDLER(INT(IRQ_TX_FIFO_OVERFLOW));
#endif
}
