#define PREAMBLE_1	0xC4CA5018
#define PREAMBLE_2	0xFAE4B982

#define SFD_1		0x565ddbca
#define SFD_2		0x58267acd

#define CRC8_POLY	0xB1	/* 10110001 = x^0 + x^2 + x^3 + x^7 + (x^8) */
#define CRC8_INIT	0xFF

#define BIT_MAP_8_0	0x55	/* 01010101 */
#define BIT_MAP_8_1	0xAA	/* 10101010 */

#define BIT_MAP_4_0	0x5	/* 0101 */
#define BIT_MAP_4_1	0xA	/* 1010 */

#define BIT_MAP_2_0	0x1	/* 01 */
#define BIT_MAP_2_1	0x2	/* 10 */

#define BIT_MAP_1_0	0x0	/* 0 */
#define BIT_MAP_1_1	0x1	/* 1 */

#define PUT_SF8_0	put_n_bits(BIT_MAP_8_0, 8)
#define PUT_SF8_1	put_n_bits(BIT_MAP_8_1, 8)

#define PUT_SF4_0	put_n_bits(BIT_MAP_4_0, 4)
#define PUT_SF4_1	put_n_bits(BIT_MAP_4_1, 4)

#define PUT_SF2_0	put_n_bits(BIT_MAP_2_0, 2)
#define PUT_SF2_1	put_n_bits(BIT_MAP_2_1, 2)

#define PUT_SF1_0	put_n_bits(BIT_MAP_1_0, 1)
#define PUT_SF1_1	put_n_bits(BIT_MAP_1_1, 1)

typedef enum {
    r_sf_8 = 0,
    r_sf_4 = 1,
    r_sf_2 = 2,
    r_sf_1 = 3
} tx_rate_t;

typedef enum {
    pilot_64 = 2,
    pilot_128 = 3
} pilot_period_t;

typedef struct {
    tx_rate_t	    data_rate;
    pilot_period_t  pilot_info;
    int		    burst_mode;
    int		    scrambler_seed; /* Fixme - check data type */
    uint8_t	    PDSU_length;
    uint8_t	    crc8;
} plcp_header_t;

extern void build_tx_preamble(void);
extern void build_tx_sfd(void);
extern void build_tx_ri(void);
extern void build_tx_plcp_header_drf(plcp_header_t *header_info);
extern void build_tx_plcp_header_ri(plcp_header_t *header_info);
extern void build_tx_payload(void);
extern void tx_trigger(void);
