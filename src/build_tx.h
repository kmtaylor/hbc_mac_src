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

#define PREAMBLE_1	0xC4CA5018
#define PREAMBLE_2	0xFAE4B982

#define SFD_1		0x565DDBCA
#define SFD_2		0x58267ACD

#define HDR_DR_SHIFT	0
#define HDR_PI_SHIFT	3
#define HDR_BM_SHIFT	8
#define HDR_SS_SHIFT	11
#define HDR_LEN_SHIFT	16
#define HDR_CRC_SHIFT	24

#define HDR_DR_MASK	0x00000007
#define HDR_PI_MASK	0x00000038
#define HDR_BM_MASK	0x00000100
#define HDR_SS_MASK	0x00000800
#define HDR_LEN_MASK	0x00ff0000
#define HDR_CRC_MASK	0xff000000

#define HDR_READ(hdr, member) \
			((hdr & HDR_##member##_MASK) >> HDR_##member##_SHIFT)

#define CRC8_POLY	0xB1	/* 10110001 = x^0 + x^2 + x^3 + x^7 + (x^8) */
#define CRC8_INIT	0xFF

#define BIT_MAP_32_0	0x55555555
#define BIT_MAP_32_1	0xAAAAAAAA

#define SCRAM_SEED_0    0x69540152
#define SCRAM_SEED_1    0x8A5F621F

#define SYM_SIZE	4
#define SYM_MASK	((1 << SYM_SIZE) - 1)
#define WALSH_SIZE	(1 << SYM_SIZE)

typedef enum {
    r_sf_64 = 0,
    r_sf_32 = 1,
    r_sf_16 = 2,
    r_sf_8  = 3
} tx_rate_t;

typedef enum {
    pilot_64 = 2,
    pilot_128 = 3,
    pilot_none = 6
} pilot_period_t;

typedef struct {
    tx_rate_t	    data_rate;
    pilot_period_t  pilot_info;
    int		    burst_mode;
    int		    use_ri;
    int		    scrambler_seed;
    uint8_t	    PDSU_length;
    uint8_t	    crc8;
} plcp_header_t;

typedef uint16_t walsh_t;

extern uint8_t crc8_update(uint8_t crc, uint8_t data);
extern uint8_t crc8(uint32_t header);

extern void build_tx_plcp_header(plcp_header_t *header_info, int checksum);
extern uint8_t build_tx_payload(plcp_header_t *header_info, int checksum);
