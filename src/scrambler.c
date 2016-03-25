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
