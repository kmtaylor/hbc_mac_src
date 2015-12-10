#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "mem.h"

#if USE_MEM
static XIOModule *mem_io_mod;

void mem_init(XIOModule *io_mod) {
    mem_io_mod = io_mod;
    /* Block until initialisation has finished */
    while (XIOModule_DiscreteRead(mem_io_mod, MEM_STATUS_GPIO) & 
		    MEM_PHY_INIT) {}
}

void mem_set_wr_p(uint32_t data) {
    XIOModule_IoWriteWord(mem_io_mod, HEX(MEM_WR_P_ADDR), data);
}

uint32_t mem_get_wr_p(void) {
    return XIOModule_IoReadWord(mem_io_mod, HEX(MEM_WR_P_ADDR));
}

void mem_set_rd_p(uint32_t data) {
    XIOModule_IoWriteWord(mem_io_mod, HEX(MEM_RD_P_ADDR), data);
}

uint32_t mem_get_rd_p(void) {
    return XIOModule_IoReadWord(mem_io_mod, HEX(MEM_RD_P_ADDR));
}

void mem_set_flags(uint8_t flags) {
    XIOModule_IoWriteByte(mem_io_mod, HEX(MEM_FLAGS_ADDR), flags);
}

uint8_t mem_get_flags(void) {
    return XIOModule_IoReadByte(mem_io_mod, HEX(MEM_FLAGS_ADDR));
}

void mem_write(uint32_t data) {
    XIOModule_IoWriteWord(mem_io_mod, HEX(MEM_RD_WR_ADDR), data);
}

uint32_t mem_read(void) {
    return XIOModule_IoReadWord(mem_io_mod, HEX(MEM_RD_WR_ADDR));
}
#else /* USE_MEM */
static uint32_t val;

void mem_init(XIOModule *io_mod) {
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
