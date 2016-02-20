#LIBS := -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
MCS_INSTANCE_NAME := mcs_0
BSP_LOCATION := ../standalone_bsp_0
HARDWARE_DESC_LOCATION := ../Genesys_64k/system.xml
HDL_LOCATION := $(HOME)/Xilinx/Projects/transceiver
HDL_BUILD_LOCATION := $(HOME)/Xilinx/Projects/transceiver_spartan
DEVICE := xc6slx9tqg144-2
BITSTREAM := toplevel.bit

PROJECT_NAME := tx_framer

INCLUDE := -I$(BSP_LOCATION)/$(MCS_INSTANCE_NAME)/include
INCLUDE += -I$(HDL_LOCATION)

CFLAGS += -mno-xl-reorder -mlittle-endian -mcpu=v8.40.a -mxl-soft-mul \
	  -ffunction-sections -fdata-sections
CFLAGS += -DUSE_MEM=1

LDFLAGS += -Wl,-T -Wl,src/lscript.ld -Wl,--gc-sections \
	   -L$(BSP_LOCATION)/$(MCS_INSTANCE_NAME)/lib

OBJS += \
	src/fifo.o \
	src/mem.o \
	src/usb.o \
	src/spi.o \
	src/flash.o \
	src/lcd.o \
	src/build_tx.o \
	src/interrupt.o \
	src/scrambler.o \
	src/extract_rx.o \
	src/main.o

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += $(PROJECT_NAME).elf.size

ELFCHECK += $(PROJECT_NAME).elf.elfcheck

# All Target
all: $(PROJECT_NAME).mcs $(PROJECT_NAME).bit secondary-outputs

# Tool invocations
$(PROJECT_NAME).elf: $(OBJS) src/lscript.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MicroBlaze gcc linker'
	mb-gcc -o "$(PROJECT_NAME).elf" $(OBJS) $(CFLAGS) $(LDFLAGS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

$(PROJECT_NAME).elf.size: $(PROJECT_NAME).elf
	@echo 'Invoking: MicroBlaze Print Size'
	mb-size $(PROJECT_NAME).elf  |tee "$(PROJECT_NAME).elf.size"
	@echo 'Finished building: $@'
	@echo ' '

$(PROJECT_NAME).elf.elfcheck: $(PROJECT_NAME).elf
	@echo 'Invoking: Xilinx ELF Check'
	elfcheck $(PROJECT_NAME).elf -hw $(HARDWARE_DESC_LOCATION) \
		-pe $(MCS_INSTANCE_NAME) | tee "$(PROJECT_NAME).elf.elfcheck"
	@echo 'Finished building: $@'
	@echo ' '

$(PROJECT_NAME).bit: $(PROJECT_NAME).elf $(HDL_BUILD_LOCATION)/$(BITSTREAM)
	data2mem -p $(DEVICE) \
	    -bm $(HDL_BUILD_LOCATION)/ipcore_dir/$(MCS_INSTANCE_NAME)_bd.bmm \
	    -bd $< tag $(MCS_INSTANCE_NAME) \
	    -bt $(HDL_BUILD_LOCATION)/$(BITSTREAM) \
	    -o b $@

$(PROJECT_NAME).mcs: $(PROJECT_NAME).bit
	impact -batch impact/gen_prom.impact
	rm _impact*
	rm $(PROJECT_NAME).cfi
	rm $(PROJECT_NAME).prm

src/%.o: src/%.c
	mb-gcc -Wall -Os -c $(INCLUDE) $(CFLAGS) -o "$@" "$<"
	@echo ' '

src/%.s: src/%.c
	mb-gcc -Wall -Os -S $(INCLUDE) $(CFLAGS) -o "$@" "$<"

# Other Targets
clean:
	-$(RM) $(OBJS) $(ELFSIZE) $(ELFCHECK) $(PROJECT_NAME).elf
	-@echo ' '

secondary-outputs: $(ELFSIZE) $(ELFCHECK)

.PHONY: all clean
.SECONDARY:
