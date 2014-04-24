#define PREAMBLE_1	0xC4CA5018
#define PREAMBLE_2	0xFAE4B982

#define SFD_1		0x565ddbca
#define SFD_2		0x58267acd

#define BIT_MAP_8_0	0x55
#define BIT_MAP_8_1	0xAA

#define BIT_MAP_4_0	0x5
#define BIT_MAP_4_1	0xA

#define BIT_MAP_2_0	0x2
#define BIT_MAP_2_1	0x1

#define BIT_MAP_1_0	0x0
#define BIT_MAP_1_1	0x1

#define PUT_SF8_0	put_n_bits(BIT_MAP_8_0, 8)
#define PUT_SF8_1	put_n_bits(BIT_MAP_8_1, 8)

typedef enum {
    r_sf_8,
    r_sf_4,
    r_sf_2,
    r_sf_1
} tx_rate_t;

extern void build_tx_preamble(void);
extern void build_tx_sfd_ri(void);
extern void build_tx_plcp_header(void);
extern void build_tx_payload(void);
