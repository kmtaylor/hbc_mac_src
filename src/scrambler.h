#define SCRAMBLER_GPIO 1

#define SCRAMBLER_RESEED		    (1 << 2)
#define SCRAMBLER_SEED_VAL		    (1 << 3)
#define SCRAMBLER_SEED_CLK		    (1 << 4)

extern void scrambler_init(XIOModule *io_mod);

extern uint32_t scrambler_read(void);

extern void scrambler_reseed(int seed);
