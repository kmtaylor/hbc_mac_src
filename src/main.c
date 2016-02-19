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

enum ctrl_state {
    CTRL_STATE_CMD,
    CTRL_STATE_REPLY,
    CTRL_STATE_ACK,
};

XIOModule io_mod;

static volatile int send_packet;

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

/* Interrupt context - interrupts are disabled */
static void ctrl_cmd(uint8_t c) {
    static uint8_t pkt[PACKET_SIZE];
    static uint8_t bytes_req;
    uint8_t crc;
    int i;

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

    /* Acknowledge successful packet */
    hbc_spi_ack(HBC_ACK);

    switch(pkt[PACKET_CMD_OFFSET]) {
	case SCRAMBLER_READ:
	    GPO_CLEAR(LED_1BIT);
	    hbc_spi_reply(scrambler_read(), 4);
	    break;
    }

    hbc_spi_reply_trigger();

#if 0
    switch (cmd) {
	/* FPGA debug commands */
	case CTRL_CMD_IRQ_STATUS_READ:
	    hbc_spi_reply(XIOModule_DiscreteRead(&io_mod, INT(IRQ_GPI)), 4);
	    break;
	case CTRL_CMD_SCRAMBLER_READ:
	    hbc_spi_reply(scrambler_read(), 4);
	    break;

	/* DRAM debug commands */
	case CTRL_CMD_MEM_READ:
	    mem_set_rd_p(0);
	    hbc_spi_reply(mem_read(), 4);
	    break;
	case CTRL_CMD_MEM_RD_ADDR:
	    CHECK_ARG;
	    hbc_spi_dump_addr(buf_to_word(cmd_arg));
	    CLEAR_ARG;
	    break;
	case CTRL_CMD_MEM_WR_ADDR:
	    CHECK_ARG;
	    hbc_spi_load_addr(buf_to_word(cmd_arg));
	    CLEAR_ARG;
	    break;
	case CTRL_CMD_MEM_DUMP:
	    CHECK_ARG;
	    hbc_spi_dump_bytes(buf_to_word(cmd_arg));
	    CLEAR_ARG;
	    break;
	case CTRL_CMD_MEM_LOAD:
	    CHECK_ARG;
	    hbc_spi_load_bytes(buf_to_word(cmd_arg));
	    CLEAR_ARG;
	    break;
	case CTRL_CMD_MEM_TEST:
	    hbc_spi_reply(mem_test(MEM_SIZE), 4);
	    break;

	/* Flash debug commands */
	case CTRL_CMD_FLASH_READ:
	    flash_read(0);
	    mem_set_rd_p(0);
	    hbc_spi_reply(CTRL_CMD_FLASH_READ, 4);
	    break;
	case CTRL_CMD_FLASH_WRITE:
	    flash_write(0, FPGA_CONFIG_SIZE);
	    hbc_spi_reply(flash_verify(0, FPGA_CONFIG_SIZE), 4);
	    break;

	/* HBC_TX commands */
	case CTRL_CMD_HBC_TX_TRIGGER:
	    send_packet = 1;
	    break;

	/* HBC_RX commands */
	case CTRL_CMD_HBC_RX_READY:
	    hbc_spi_reply(rx_packet_ready(), 4);
	    break;
	case CTRL_CMD_HBC_RX_LENGTH:
	    hbc_spi_reply(rx_packet_length(), 4);
	    break;
	case CTRL_CMD_HBC_RX_BYTES_READ:
	    hbc_spi_reply(rx_bytes_read(), 4);
	    break;
	case CTRL_CMD_HBC_RX_CRC_OK:
	    hbc_spi_reply(rx_check_crc_ok(), 4);
	    break;
	case CTRL_CMD_HBC_RX_READ:
	    hbc_spi_reply(rx_read(), 4);
	    break;
	case CTRL_CMD_HBC_RX_NEXT:
	    rx_packet_next();
	    break;
	case CTRL_CMD_HBC_RX_CHECK:
	    hbc_spi_reply(rx_check_packet(), 4);
	    break;
    }
#endif
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

    GPO_SET(LED_1BIT);

    plcp_header_t header_info = {
	.data_rate = r_sf_8,
        .pilot_info = pilot_none,
        .burst_mode = 0,
	.use_ri = 1,
        .scrambler_seed = 0,
        .PDSU_length = 254,
    };

    while (1) {
	if (send_packet) {

	    disable_interrupts();
	    mem_set_rd_p(0);
	    build_tx_plcp_header(&header_info);
	    build_tx_payload(&header_info);
	    enable_interrupts();

	    send_packet = 0;
	}
    }

    return 0;
}
