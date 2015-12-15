#include <stdint.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "scrambler.h"

uint32_t scrambler_read(void) {
    return XIOModule_IoReadWord(&io_mod, HEX(SCRAMBLER_ADDR));
}

void scrambler_reseed(int seed) {
    XIOModule_DiscreteSet(&io_mod, SCRAMBLER_GPIO, 
	    seed ? SCRAMBLER_RESEED | SCRAMBLER_SEED_VAL : SCRAMBLER_RESEED);
    XIOModule_DiscreteSet(&io_mod, SCRAMBLER_GPIO, SCRAMBLER_SEED_CLK);
    XIOModule_DiscreteClear(&io_mod, SCRAMBLER_GPIO,
	    SCRAMBLER_RESEED | SCRAMBLER_SEED_VAL | SCRAMBLER_SEED_CLK);
}
