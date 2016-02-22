/* M25P40 4MB Flash device
 * 512KB = (8 * 64KB sectors) = (2048 * 256B pages)
 * Maximum address: 0x7ffff
 * FPGA configuration data occupies:
 * 0x00000 to 0x5327B
 *
 * There is 183684 bytes remaining. Enough to store the PSoC flash.
 * 
 * Memory map:
 *
 * Start addr	End addr    Size    Purpose
 *
 * 0x00000	0x5327B	    340604  FPGA configuration
 * 0x60000	0x60003	    4	    PSOC JTAG ID
 * 0x60004	0x60005	    2	    PSOC Image checksum
 * 0x60008	0x6000B	    4	    PSOC NVL (non-volatile latch)
 * 0x6000C	0x6000F	    4	    PSOC WOL (write once NV latch)
 * 0x60010	0x6002F	    32	    PSOC Flash protection data
 * 0x60030	0x6902F	    36864   PSOC Flash data
 */

#define FLASH_SIZE		    (512 * 1024)
#define PAGE_SIZE		    256
#define SECTOR_SIZE		    (256 * 256)

#define FLASH_ADDR_FPGA_CONF	    0x00000
#define FLASH_ADDR_PSOC_CONF	    0x60000

#define FLASH_ADDR_PSOC_JTAG	    FLASH_ADDR_PSOC_CONF
#define FLASH_ADDR_PSOC_CHKSUM	    (FLASH_ADDR_PSOC_CONF + 0x04)
#define FLASH_ADDR_PSOC_NVL	    (FLASH_ADDR_PSOC_CONF + 0x08)
#define FLASH_ADDR_PSOC_WOL	    (FLASH_ADDR_PSOC_CONF + 0x0C)
#define FLASH_ADDR_PSOC_PROTECT	    (FLASH_ADDR_PSOC_CONF + 0x10)
#define FLASH_ADDR_PSOC_FLASH	    (FLASH_ADDR_PSOC_CONF + 0x30)
#define FLASH_PSOC_ROW_SIZE	    288
#define FLASH_PSOC_ROW_COUNT	    128
#define FLASH_PSOC_SIZE		    36912

#define FPGA_CONFIG_SIZE	    340604

#define FLASH_WRITE_ENABLE	    0x06
#define FLASH_WRITE_DISABLE	    0x04
#define FLASH_READ_ID		    0x9F
#define FLASH_READ_STATUS	    0x05
#define FLASH_WRITE_STATUS	    0x01
#define FLASH_READ_DATA		    0x03
#define FLASH_READ_DATA_HS	    0x0B
#define FLASH_PAGE_PROGRAM	    0x02
#define FLASH_SECTOR_ERASE	    0xD8
#define FLASH_BULK_ERASE	    0xC7
#define FLASH_POWER_DOWN	    0xB9
#define FLASH_POWER_UP		    0xAB

#define FLASH_STATUS_WIP	    0x01
#define FLASH_STATUS_WEL	    0x02

extern void flash_init(void);

extern void flash_start(void);
extern void flash_end(void);
extern uint8_t flash_transfer(uint8_t data);

extern uint32_t flash_get_id(void);
extern void flash_read(uint32_t mem_addr, uint32_t size, uint32_t flash_addr);
extern void flash_write(uint32_t mem_addr, uint32_t size, uint32_t flash_addr);
extern int flash_verify(uint32_t mem_addr, uint32_t size, uint32_t flash_addr);

extern uint8_t flash_read_status(void);

static inline uint32_t flash_mem_to_page(uint32_t flash_addr) {
    return flash_addr / PAGE_SIZE;
}

static inline uint32_t flash_mem_to_sector(uint32_t flash_addr) {
    return flash_addr / SECTOR_SIZE;
}

static inline uint32_t flash_num_pages(uint32_t size) {
    uint32_t pages;
    pages = size / PAGE_SIZE;
    if (size % PAGE_SIZE) pages++;
    return pages;
}

static inline uint32_t flash_num_sectors(uint32_t size) {
    uint32_t sectors;
    sectors = size / SECTOR_SIZE;
    if (size % SECTOR_SIZE) sectors++;
    return sectors;
}
