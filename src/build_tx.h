#define PREAMBLE_1	0xC4CA5018
#define PREAMBLE_2	0xFAE4B982

#define SFD_1		0x565ddbca
#define SFD_2		0x58267acd

#define HDR_DR_SHIFT	0
#define HDR_PI_SHIFT	3
#define HDR_BM_SHIFT	8
#define HDR_SS_SHIFT	11
#define HDR_LEN_SHIFT	16
#define HDR_CRC_SHIFT	24

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

extern void build_tx_plcp_header(plcp_header_t *header_info);
extern void build_tx_payload(plcp_header_t *header_info);
extern void tx_trigger(void);
