#include <stdint.h>
#include <string.h>
#include <xiomodule.h>
#include <preprocessor/constants.vhh>

#include "gpio.h"
#include "lcd.h"
#include "fifo.h"
#include "usb.h"
#include "spi.h"
#include "host_protocol.h"
#include "flash.h"
#include "mem.h"
#include "build_tx.h"
#include "scrambler.h"
#include "interrupt.h"
#include "extract_rx.h"

#include "../cypress/psoc_flash.h"

#define LOOPBACK	    1
#define PSOC_FLASH_ON_BOOT  0

enum ctrl_state {
    CTRL_STATE_CMD,
    CTRL_STATE_REPLY,
    CTRL_STATE_ACK,
};

XIOModule io_mod;

static volatile int send_packet;
static volatile int tx_pending;
static uint32_t pkt_addr;
static volatile int rx_auto, tx_auto;

static volatile int do_psoc_flash = PSOC_FLASH_ON_BOOT;

static plcp_header_t header_info = {
    .data_rate = r_sf_64,
    .pilot_info = pilot_none,
    .burst_mode = 0,
    .use_ri = 1,
    .scrambler_seed = 0,
};

static int rx_check_packet(void) {
    int bytes, correct = 0;
    uint32_t data, chk;

    if (!rx_packet_ready()) return -1;

    if (!rx_check_crc_ok()) goto discard_packet;
	
    bytes = rx_packet_length();
	
    if (rx_bytes_read() < bytes) goto discard_packet;

    /* Ignore packet marker and header */
    data = rx_read();
    data = rx_read();

    mem_set_rd_p(0);
    chk = mem_read();
    data = rx_read();
    while (bytes) {
	if ((chk & 0xff) == (data & 0xff)) correct++;
	chk >>= 8;
	data >>= 8;
	bytes--;
	if (bytes && ((bytes % 4) == 0)) {
	    chk = mem_read();
	    data = rx_read();
	}
    }

    rx_packet_next();
    return correct;

discard_packet:
    rx_packet_next();
    return -1;
}

static uint32_t buf_to_word(uint8_t *buf) {
    return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
}

static void reply_pkt(uint8_t cmd, uint8_t arg) {
    uint32_t data;
    uint8_t crc = CRC8_INIT;

    data = PACKET_HEADER;
    data |= cmd << 8;
    data |= arg << 16;
    crc = crc8(data);
    data |= crc << 24;

    hbc_spi_ack(data);
}

/* Interrupt context - interrupts are disabled */
static void ctrl_cmd(uint8_t c) {
    static uint32_t flash_addr;
    static uint8_t pkt[PACKET_SIZE];
    static uint8_t bytes_req;
    uint32_t arg;
    uint8_t crc;
    int i;
    int do_ack = HBC_ACK;

    if (!bytes_req) {
	if (c == PACKET_HEADER) bytes_req = PACKET_SIZE;
        return;
    }

    pkt[PACKET_SIZE - bytes_req] = c;
    bytes_req--;

    /* Packet not finished yet */
    if (bytes_req) return;

    /* Check for CRC error */
    crc = CRC8_INIT;
    for (i = 0; i < PACKET_SIZE; i++) {
        crc = crc8_update(crc, pkt[i]);
    }
    if (crc) return;

    arg = buf_to_word(&pkt[PACKET_ARG_OFFSET]);

    switch(pkt[PACKET_CMD_OFFSET]) {
	/* FPGA debug commands */
	case CMD_IRQ_STATUS_READ:
	    hbc_spi_reply(XIOModule_DiscreteRead(&io_mod, INT(IRQ_GPI)), 4);
	    break;
	case CMD_SCRAMBLER_READ:
	    hbc_spi_reply(scrambler_read(), 4);
	    break;

	/* DRAM debug commands */
	case CMD_MEM_READ:
	    mem_set_rd_p(arg);
	    hbc_spi_reply(mem_read(), 4);
	    break;
	case CMD_MEM_RD_ADDR:
	    hbc_spi_dump_addr(arg);
	    break;
	case CMD_MEM_WR_ADDR:
	    hbc_spi_load_addr(arg);
	    break;
	case CMD_MEM_DUMP:
	    hbc_spi_dump_bytes(arg, 0);
	    break;
	case CMD_MEM_LOAD:
	    hbc_spi_load_bytes(arg, 0);
	    break;
	case CMD_MEM_TEST:
	    hbc_spi_reply(mem_test(MEM_SIZE), 4);
	    break;

	/* Flash debug commands */
	case CMD_FLASH_READ:
	    flash_read(0, FLASH_SIZE, 0);
	    hbc_spi_reply(CMD_FLASH_READ, 4);
	    break;
	case CMD_FLASH_WRITE_ADDR:
	    flash_addr = arg;
	    break;
	case CMD_FLASH_WRITE:
	    flash_write(0, arg, flash_addr);
	    hbc_spi_reply(flash_verify(0, arg, flash_addr), 4);
	    break;

	/* PSoC flash commands */
	case CMD_PSOC_FLASH:
	    do_psoc_flash = 1;
	    break;

	/* HBC_TX commands */
	case CMD_HBC_TX_TRIGGER:
	    header_info.PDSU_length = arg;
	    send_packet = 1;
	    break;
	case CMD_HBC_TX_SPREAD_FACTOR:
	    header_info.data_rate = arg;
	    break;
	case CMD_HBC_TX_SCRAMBLER_SEED:
	    header_info.scrambler_seed = arg;
	    break;
	case CMD_HBC_TX_AUTO:
	    tx_auto = arg;
	    break;
	case CMD_HBC_TX_ADDR:
	    pkt_addr = arg;
	    hbc_spi_load_addr(arg);
	    break;
	case CMD_HBC_TX_PACKET:
	    hbc_spi_load_bytes(arg, 1);
	    do_ack = NO_ACK;
	    tx_pending++;
	    break;

	/* HBC_RX commands */
	case CMD_HBC_RX_READY:
	    hbc_spi_reply(rx_packet_ready(), 4);
	    break;
	case CMD_HBC_RX_LENGTH:
	    hbc_spi_reply(rx_packet_length(), 4);
	    break;
	case CMD_HBC_RX_BYTES_READ:
	    hbc_spi_reply(rx_bytes_read(), 4);
	    break;
	case CMD_HBC_RX_CRC_OK:
	    hbc_spi_reply(rx_check_crc_ok(), 4);
	    break;
	case CMD_HBC_RX_READ:
	    hbc_spi_reply(rx_read(), 4);
	    break;
	case CMD_HBC_RX_NEXT:
	    rx_packet_next();
	    break;
	case CMD_HBC_RX_CHECK:
	    hbc_spi_reply(rx_check_packet(), 4);
	    break;
	case CMD_HBC_RX_AUTO:
	    rx_auto = arg;
	    break;
	case CMD_HBC_RX_GET_ADDR:
	    hbc_spi_reply(rx_read_addr(), 4);
	    break;

	default:
	    /* If pass through is enabled, pretend that we are the PSOC */
	    do_ack = PSOC_ACK;
	    break;
    }
    
    /* Acknowledge successful packet */
    if (do_ack != NO_ACK)
	reply_pkt(do_ack, 0);

    hbc_spi_data_trigger();
}

#define RX_PAD 4
static void hbc_to_spi(void) {
    int bytes;

    if (!rx_packet_ready()) return;

    if (!rx_check_crc_ok()) goto discard_packet;
	
    static int toggle;
    toggle ? GPO_CLEAR(LED_1BIT) : GPO_SET(LED_1BIT);
    toggle = !toggle;
    bytes = rx_packet_length();
	
    if (rx_bytes_read() < bytes) goto discard_packet;

    /* Send packet ready command */
    reply_pkt(RX_PACKET, bytes);
    
    /* Skip packet marker */
    rx_read();
    /* Dump packet and header */
    hbc_spi_dump_bytes(bytes + HBC_HEADER_SIZE + RX_PAD, 1);
    hbc_spi_data_trigger();

discard_packet:
    rx_packet_next();
    return;
}

int main() {
    XIOModule_Initialize(&io_mod, XPAR_IOMODULE_0_DEVICE_ID);

    fifo_init();
    mem_init();
    flash_init();
    rx_init();
    hbc_spi_init(ctrl_cmd);

    /* Divide memory into RX and TX circular buffers */
    mem_set_flags(MEM_READ_WRAP_16M | MEM_WRITE_WRAP_16M);

    setup_interrupts();
    ENABLE_INTERRUPT(INT(IRQ_RX_FIFO_ALMOST_FULL));
    ENABLE_INTERRUPT(INT(IRQ_RX_PKT_READY));
    ENABLE_INTERRUPT(INT(IRQ_HBC_CTRL_SPI));
    ENABLE_INTERRUPT(INT(IRQ_HBC_DATA_SPI));
    enable_interrupts();

    GPO_SET(HBC_DATA_SWITCH);
    rx_enable();

    GPO_CLEAR(LED_1BIT);

    while (1) {
	if (send_packet) {
	    /* Send a single packet starting at memory address 0 */
	    disable_interrupts();
	    mem_set_rd_p(0);
	    build_tx_plcp_header(&header_info);
	    build_tx_payload(&header_info);
	    enable_interrupts();
	    /* The scrambler seed is to be toggled each frame (10.7.1) */
	    header_info.scrambler_seed = !header_info.scrambler_seed;

	    send_packet = 0;
	}

	if (tx_auto && (
		(tx_pending > 1) || 
		((tx_pending == 1) && !hbc_spi_load_busy()))) {
	    /* Check TX circular buffer for a packet to send */
#if !LOOPBACK
	    rx_disable();
#endif
	    disable_interrupts();
	    mem_set_rd_p(pkt_addr);
	    header_info.PDSU_length = mem_read();
	    build_tx_plcp_header(&header_info);
	    pkt_addr += (build_tx_payload(&header_info) * 4) + 4;
	    enable_interrupts();
	    pkt_addr &= MEM_TX_MASK;
	    /* The scrambler seed is to be toggled each frame (10.7.1) */
	    header_info.scrambler_seed = !header_info.scrambler_seed;
#if !LOOPBACK
	    rx_enable();
#endif

	    tx_pending--;
	}

	if (rx_auto && !hbc_spi_dump_busy()) {
	    /* Check RX circular buffer for a packet to send */
	    hbc_to_spi();
	}

	if (do_psoc_flash) {
	    disable_interrupts();
	    psoc_flash_device();
	    enable_interrupts();
	    do_psoc_flash = 0;
	}
    }
}
