#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "interrupt.h"
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
