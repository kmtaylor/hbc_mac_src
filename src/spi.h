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

extern void hbc_spi_init(void (*fn)(uint8_t));

extern void hbc_spi_reply(uint32_t data, int size);
extern void hbc_spi_ack(uint32_t ack_cmd);

extern void hbc_spi_dump_addr(uint32_t addr);
extern void hbc_spi_load_addr(uint32_t addr);

extern void hbc_spi_dump_bytes(uint32_t bytes, int from_rx);
extern void hbc_spi_load_bytes(uint32_t bytes, int for_tx);

extern int hbc_spi_load_busy(void);
extern int hbc_spi_dump_busy(void);

extern void hbc_spi_data_trigger(void);
