extern XIOModule io_mod;

#define GPO_SET(gpo) \
    XIOModule_DiscreteSet(&io_mod, INT(GPP_##gpo), (1 << INT(GPO_##gpo)))

#define GPO_CLEAR(gpo) \
    XIOModule_DiscreteClear(&io_mod, INT(GPP_##gpo), (1 << INT(GPO_##gpo)))

#define GPO_OUT(gpo) \
    XIOModule_DiscreteSet(&io_mod, INT(GPP_##gpo##_DIR),		    \
                            (1 << INT(GPO_##gpo##_DIR)))
#define GPO_IN(gpo) \
    XIOModule_DiscreteClear(&io_mod, INT(GPP_##gpo##_DIR),		    \
                            (1 << INT(GPO_##gpo##_DIR)))

#define GPI_READ(gpi) \
    (XIOModule_DiscreteRead(&io_mod, INT(GPP_##gpi)) & (1 << INT(GPI_##gpi)))
