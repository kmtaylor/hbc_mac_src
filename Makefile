# Copyright (C) 2016 Kim Taylor
#
# This file is part of hbc_mac.
#
# hbc_mac is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# hbc_mac is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with hbc_mac.  If not, see <http://www.gnu.org/licenses/>.

#LIBS := -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
MCS_INSTANCE := mcs_0
BSP_LOCATION := ../standalone_bsp_0
HARDWARE_DESC_LOCATION := ../Genesys_64k/system.xml
HDL_LOCATION := $(HOME)/Xilinx/Projects/transceiver
HDL_BUILD_LOCATION := $(HOME)/Xilinx/Projects/transceiver/build
IP_CORE_LOCATION := $(HDL_BUILD_LOCATION)/cores
MCS_MEM_MAP := $(IP_CORE_LOCATION)/$(MCS_INSTANCE)/$(MCS_INSTANCE)_bd.bmm
DEVICE := xc6slx9tqg144-2
BITSTREAM := hbc_mac.bit

PROJECT_NAME := hbc_mac

INCLUDE := -I$(BSP_LOCATION)/$(MCS_INSTANCE)/include
INCLUDE += -I$(HDL_LOCATION)

CFLAGS += -mno-xl-reorder -mlittle-endian -mcpu=v8.40.a -mxl-soft-mul \
	  -ffunction-sections -fdata-sections
CFLAGS += -DUSE_MEM=1

LDFLAGS += -Wl,-T -Wl,src/lscript.ld -Wl,--gc-sections \
	   -L$(BSP_LOCATION)/$(MCS_INSTANCE)/lib

PSOC_FLASH_OBJS += \
	cypress/ProgrammingSteps.o \
	cypress/SWD_PacketLayer.o \
	cypress/SWD_PhysicalLayer.o \
	cypress/DataFetch.o \
	cypress/DeviceAcquire.o \
	cypress/Timeout.o \
	cypress/TimeoutCalc.o \
	cypress/psoc_flash.o
	   
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
all: $(PROJECT_NAME).bin

# Tool invocations
$(PROJECT_NAME).elf: $(OBJS) src/lscript.ld $(PSOC_FLASH_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MicroBlaze gcc linker'
	mb-gcc -o "$(PROJECT_NAME).elf" $(OBJS) $(PSOC_FLASH_OBJS) \
		$(CFLAGS) $(LDFLAGS) $(LIBS)
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
		-pe $(MCS_INSTANCE) | tee "$(PROJECT_NAME).elf.elfcheck"
	@echo 'Finished building: $@'
	@echo ' '

$(PROJECT_NAME).bit: $(PROJECT_NAME).elf $(HDL_BUILD_LOCATION)/$(BITSTREAM)
	data2mem -p $(DEVICE) \
	    -bm $(MCS_MEM_MAP) \
	    -bd $< tag $(MCS_INSTANCE) \
	    -bt $(HDL_BUILD_LOCATION)/$(BITSTREAM) \
	    -o b $@

$(PROJECT_NAME).mcs: $(PROJECT_NAME).bit
	impact -batch impact/gen_prom.impact > /dev/null
	rm _impact*
	rm $(PROJECT_NAME).cfi
	rm $(PROJECT_NAME).prm

%.bin: %.mcs
	ihex2bin $<

%.o: %.c
	mb-gcc -Wall -Os -c $(INCLUDE) $(CFLAGS) -o "$@" "$<"
	@echo ' '

%.s: %.c
	mb-gcc -Wall -Os -S $(INCLUDE) $(CFLAGS) -o "$@" "$<"

# Other Targets
clean:
	-$(RM) $(OBJS) $(PSOC_FLASH_OBJS) $(ELFSIZE) $(ELFCHECK)
	-$(RM) $(PROJECT_NAME).elf
	-$(RM) $(PROJECT_NAME).bin
	-$(RM) $(PROJECT_NAME).mcs
	-$(RM) $(PROJECT_NAME).bit
	-@echo ' '

secondary-outputs: $(ELFSIZE) $(ELFCHECK)

.PHONY: all clean
.SECONDARY:
