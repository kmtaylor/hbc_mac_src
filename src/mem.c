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
#include "interrupt.h"
#include "scrambler.h"
#include "mem.h"

#if USE_MEM
void mem_init(void) {
    /* Block until initialisation has finished */
    while (IRQ_FLAG_SET(IRQ_RAM_INIT)) {}
}

void mem_set_wr_p(uint32_t data) {
    XIOModule_IoWriteWord(&io_mod, HEX(MEM_WR_P_ADDR), data);
}

uint32_t mem_get_wr_p(void) {
    return XIOModule_IoReadWord(&io_mod, HEX(MEM_WR_P_ADDR));
}

void mem_set_rd_p(uint32_t data) {
    XIOModule_IoWriteWord(&io_mod, HEX(MEM_RD_P_ADDR), data);
}

uint32_t mem_get_rd_p(void) {
    return XIOModule_IoReadWord(&io_mod, HEX(MEM_RD_P_ADDR));
}

void mem_set_flags(uint8_t flags) {
    XIOModule_IoWriteByte(&io_mod, HEX(MEM_FLAGS_ADDR), flags);
}

uint8_t mem_get_flags(void) {
    return XIOModule_IoReadByte(&io_mod, HEX(MEM_FLAGS_ADDR));
}

void mem_write(uint32_t data) {
    XIOModule_IoWriteWord(&io_mod, HEX(MEM_RD_WR_ADDR), data);
}

uint32_t mem_read(void) {
    return XIOModule_IoReadWord(&io_mod, HEX(MEM_RD_WR_ADDR));
}

uint8_t mem_test(uint32_t bytes) {
    uint32_t i, mem, scram;
    uint32_t errors = 0;
    uint8_t flags;

    flags = mem_get_flags();
    mem_set_flags(0);
    mem_set_wr_p(0);
    scrambler_reseed(0);

    for (i = 0; i < bytes/4; i++) {
        mem_write(scrambler_read());
    }
    
    mem_set_rd_p(0);
    mem_set_wr_p(0);
    scrambler_reseed(0);

    for (i = 0; i < bytes/4; i++) {
        mem = mem_read();
        scram = scrambler_read();
        if (mem != scram) errors++;
    }
    
    mem_set_rd_p(0);
    mem_set_wr_p(0);
    scrambler_reseed(0);
    mem_set_flags(flags);

    return errors > 0xff ? 0xff : errors;
}
#else /* USE_MEM */
static uint32_t val;

void mem_init(void) {
}

void mem_set_wr_p(uint32_t data) {
}

uint32_t mem_get_wr_p(void) {
    return 0;
}

void mem_set_rd_p(uint32_t data) {
    val = 0;
}

uint32_t mem_get_rd_p(void) {
    return 0;
}

void mem_set_flags(uint8_t flags) {
}

uint8_t mem_get_flags(void) {
    return 0;
}

void mem_write(uint32_t data) {
}

uint32_t mem_read(void) {
    return val++;
}
#endif /* USE_MEM */
