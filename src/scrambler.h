/* Copyright (C) 2016 Kim Taylor
 *
 * This file is part of hbc_mac.
 *
 * hbc_mac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with hbc_mac.  If not, see <http://www.gnu.org/licenses/>.
 */

#define SCRAMBLER_GPIO			    INT(SCRAMBLER_GPIO)

#define SCRAMBLER_RESEED		    (1 << INT(GPO_SCRAM_RESEED))
#define SCRAMBLER_SEED_VAL		    (1 << INT(GPO_SCRAM_SEED_VAL))
#define SCRAMBLER_SEED_CLK		    (1 << INT(GPO_SCRAM_SEED_CLK))

extern uint32_t scrambler_read(void);

extern void scrambler_reseed(int seed);
