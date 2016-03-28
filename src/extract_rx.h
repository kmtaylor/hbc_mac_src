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

extern void rx_init(void);

extern int rx_packet_ready(void);
extern uint8_t rx_packet_length(void);
extern int rx_check_packet(int checksum);
extern int rx_check_crc_ok(void);
extern uint32_t rx_read(void);
extern void rx_packet_next(void);
extern uint32_t rx_bytes_read(void);
extern void rx_enable(void);
extern void rx_disable(void);

extern uint32_t rx_read_addr(void);
extern uint32_t rx_write_addr(void);
