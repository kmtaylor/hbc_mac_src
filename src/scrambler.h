#define SCRAMBLER_GPIO			    INT(HBC_GPIO)

#define SCRAMBLER_RESEED		    (1 << INT(GPO_SCRAM_RESEED))
#define SCRAMBLER_SEED_VAL		    (1 << INT(GPO_SCRAM_SEED_VAL))
#define SCRAMBLER_SEED_CLK		    (1 << INT(GPO_SCRAM_SEED_CLK))

extern void scrambler_init(XIOModule *io_mod);

extern uint32_t scrambler_read(void);

extern void scrambler_reseed(int seed);
