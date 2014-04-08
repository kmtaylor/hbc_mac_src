#define PREAMBLE_1	0xC4CA5018
#define PREAMBLE_2	0xFAE4B982

#define BIT_MAP_8_0	0x55
#define BIT_MAP_8_1	0xAA

extern void build_tx_preamble(void);
extern void build_tx_sfd_ri(void);
extern void build_tx_plcp_header(void);
extern void build_tx_payload(void);
