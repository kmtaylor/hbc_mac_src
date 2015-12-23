/* M25P40 4MB Flash device
 * 512Kb = (8 * 64Kb sectors) = (2048 * 256b pages)
 * Maximum address: 0x7ffff
 */

#define FLASH_SIZE		    (512 * 1024)
#define PAGE_SIZE		    256

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
extern void flash_read(uint32_t mem_addr);
extern void flash_write(uint32_t mem_addr, uint32_t size);
extern int flash_verify(uint32_t mem_addr, uint32_t size);

extern uint8_t flash_read_status(void);
