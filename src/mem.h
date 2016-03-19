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

#define MEM_SIZE	    (1024*1024*32)
#define MEM_TX_BUF	    0
#define MEM_RX_BUF	    (MEM_SIZE/2)
#define MEM_TX_MASK	    (MEM_RX_BUF-1)

#define MEM_READ_WRAP_16M   0x02
#define MEM_WRITE_WRAP_16M  0x01

extern void mem_init(void);

extern void mem_set_wr_p(uint32_t data);
extern void mem_set_rd_p(uint32_t data);
extern uint32_t mem_get_wr_p(void);
extern uint32_t mem_get_rd_p(void);

void mem_set_flags(uint8_t flags);
uint8_t mem_get_flags(void);

extern void mem_write(uint32_t data);
extern uint32_t mem_read(void);

extern uint8_t mem_test(uint32_t bytes);
