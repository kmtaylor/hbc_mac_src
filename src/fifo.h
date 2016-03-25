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

extern void fifo_init(void);

extern void fifo_write(uint32_t data);
extern void fifo_write_size(uint8_t size);
extern uint32_t fifo_read(void);

extern void fifo_trigger(void);
extern void fifo_flush(void);
extern void fifo_reset(void);
extern void fifo_wait(void);

extern void fifo_modulate_sf(uint8_t data);
extern void fifo_modulate(uint32_t data);
