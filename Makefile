#LIBS := -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
INCLUDE := -I../standalone_bsp_0/mcs_0/include

CFLAGS += -mno-xl-reorder -mlittle-endian -mcpu=v8.40.a -mxl-soft-mul \
	  -ffunction-sections -fdata-sections

LDFLAGS += -Wl,-T -Wl,src/lscript.ld -Wl,--gc-sections \
	   -L../standalone_bsp_0/mcs_0/lib

OBJS += \
	src/fifo.o \
	src/mem.o \
	src/usb.o \
	src/lcd.o \
	src/build_tx.o \
	src/interrupt.o \
	src/main.o

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += tx_framer.elf.size

ELFCHECK += tx_framer.elf.elfcheck

# All Target
all: tx_framer.elf secondary-outputs

# Tool invocations
tx_framer.elf: $(OBJS) src/lscript.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MicroBlaze gcc linker'
	mb-gcc -o "tx_framer.elf" $(OBJS) $(CFLAGS) $(LDFLAGS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

tx_framer.elf.size: tx_framer.elf
	@echo 'Invoking: MicroBlaze Print Size'
	mb-size tx_framer.elf  |tee "tx_framer.elf.size"
	@echo 'Finished building: $@'
	@echo ' '

tx_framer.elf.elfcheck: tx_framer.elf
	@echo 'Invoking: Xilinx ELF Check'
	elfcheck tx_framer.elf -hw ../Genesys_64k/system.xml -pe mcs_0  \
		| tee "tx_framer.elf.elfcheck"
	@echo 'Finished building: $@'
	@echo ' '

src/%.o: src/%.c
	mb-gcc -Wall -Os -c $(INCLUDE) $(CFLAGS) -o "$@" "$<"
	@echo ' '

src/%.s: src/%.c
	mb-gcc -Wall -Os -S $(INCLUDE) $(CFLAGS) -o "$@" "$<"

# Other Targets
clean:
	-$(RM) $(OBJS) $(ELFSIZE) $(ELFCHECK) tx_framer.elf
	-@echo ' '

secondary-outputs: $(ELFSIZE) $(ELFCHECK)

.PHONY: all clean
.SECONDARY:
