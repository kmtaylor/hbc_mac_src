#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "scrambler.h"

static XIOModule *scrambler_io_mod;

void scrambler_init(XIOModule *io_mod) {
    scrambler_io_mod = io_mod;
}

uint32_t scrambler_read(void) {
    return XIOModule_IoReadWord(scrambler_io_mod, HEX(SCRAMBLER_ADDR));
}

void scrambler_reseed(int seed) {
    XIOModule_DiscreteSet(scrambler_io_mod, SCRAMBLER_GPIO, 
	    seed ? SCRAMBLER_RESEED | SCRAMBLER_SEED_VAL : SCRAMBLER_RESEED);
    XIOModule_DiscreteSet(scrambler_io_mod, SCRAMBLER_GPIO, SCRAMBLER_SEED_CLK);
    XIOModule_DiscreteClear(scrambler_io_mod, SCRAMBLER_GPIO,
	    SCRAMBLER_RESEED | SCRAMBLER_SEED_VAL | SCRAMBLER_SEED_CLK);
}
