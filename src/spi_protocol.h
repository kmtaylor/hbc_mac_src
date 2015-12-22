enum ctrl_status {
    CTRL_STATUS_EMPTY	    = 0x00,
    CTRL_STATUS_OK	    = 0x80,
    CTRL_STATUS_CMD_DONE    = 0x40,
};

enum ctrl_command {
    CTRL_CMD_DATA_STATUS    = 0x01,
    CTRL_CMD_DATA           = 0x02,
    CTRL_CMD_DATA_NEXT      = 0x03,

    CTRL_CMD_READ_SCRAMBLER = 0x80,
    CTRL_CMD_READ_MEM	    = 0x81,
    CTRL_CMD_TEST_MEM	    = 0x82,
};
