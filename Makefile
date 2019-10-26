##########################################################################################################################
# gd32vf103 GCC compiler Makefile
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
# ------------------------------------------------

######################################
# target
######################################
TARGET = gd32vf103
######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


PROJECTBASE = $(PWD)
override PROJECTBASE    := $(abspath $(PROJECTBASE))
TOP_DIR = $(PROJECTBASE)


#######################################
# binaries
#######################################
PREFIX    = $(TOP_DIR)/tools/riscv-none-gcc/8.2.0-2.2-20190521-0004/bin/riscv-none-embed-
CC        = $(PREFIX)gcc
AS        = $(PREFIX)gcc -x assembler-with-cpp
OBJCOPY   = $(PREFIX)objcopy
OBJDUMP   = $(PREFIX)objdump
AR        = $(PREFIX)ar
SZ        = $(PREFIX)size
LD        = $(PREFIX)ld
HEX       = $(OBJCOPY) -O ihex
BIN       = $(OBJCOPY) -O binary -S
#GDB       = $(PREFIX)gdb
GDB       = /opt/riscv-none-eabi-insight/bin/riscv-none-eabi-insight #使用insight代替gdb来调试

#######################################
# paths
#######################################
# firmware library path
PERIFLIB_PATH =

# Build path
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

######################################
# source
######################################
# C sources
C_SOURCES =  \
		${wildcard $(TOP_DIR)/GD32VF103_Firmware_Library/GD32VF103_standard_peripheral/*.c} \
		${wildcard $(TOP_DIR)/GD32VF103_Firmware_Library/GD32VF103_standard_peripheral/Source/*.c} \
		${wildcard $(TOP_DIR)/GD32VF103_Firmware_Library/RISCV/drivers/*.c} \
		${wildcard $(TOP_DIR)/GD32VF103_Firmware_Library/RISCV/env_Eclipse/*.c} \
		${wildcard $(TOP_DIR)/GD32VF103_Firmware_Library/RISCV/stubs/*.c} \
		${wildcard $(TOP_DIR)/freertos/*.c} \
		${wildcard $(TOP_DIR)/freertos/portable/GCC/N200/*.c} \
		${wildcard $(TOP_DIR)/freertos/portable/MemMang/heap_4.c} \
		${wildcard $(TOP_DIR)/Application/*.c} \

# ASM sources
ASM_SOURCES =  \
		${wildcard $(TOP_DIR)/GD32VF103_Firmware_Library/RISCV/env_Eclipse/*.S} \
		${wildcard $(TOP_DIR)/freertos/portable/GCC/N200/*.S}

######################################
# firmware library
######################################
PERIFLIB_SOURCES =


#######################################
# CFLAGS
#######################################
# cpu
# 可选 rv32i[m][a][f[d]][c]
#      rv32g[c]
#  	   rv64i[m][a][f[d]][c]
#      rv64g[c]
#      i为通用指令，m为整数乘法法指令，a为原子操作指令，f为单精度浮点指令，d为双精度浮点指令，c为16位压缩指令
#      imafd合称g，即通用组合指令
# GD32的cpu指令集为rv32imac
CPU = -march=rv32imac
# abi
# 可选 ilp32
#      ilp32f
#      ilp32d
#      lp64
#      lp64f
#      lp64d
#      ilp32代表int、long为32位
#      lp64代表int为32位，long为64位
#      f为单精度浮点abi，d为双精度浮点abi
# GD32应使用ilp32
ABI = -mabi=ilp32
# mcu
MCU = $(CPU) $(ABI) -mcmodel=medlow -msmall-data-limit=8 -fmessage-length=0 -fsigned-char

# macros for gcc
# AS defines
AS_DEFS = \
		-DGD32VF103C_START \
		-DUSE_STDPERIPH_DRIVER

# C defines
C_DEFS = \
		-DGD32VF103C_START \
		-DUSE_STDPERIPH_DRIVER

# AS includes
AS_INCLUDES = \
		-I $(TOP_DIR)/GD32VF103_Firmware_Library/RISCV/drivers

# C includes
C_INCLUDES = \
		-I $(TOP_DIR)/GD32VF103_Firmware_Library/GD32VF103_standard_peripheral  \
		-I $(TOP_DIR)/GD32VF103_Firmware_Library/GD32VF103_standard_peripheral/Include  \
		-I $(TOP_DIR)/GD32VF103_Firmware_Library/RISCV/drivers  \
		-I $(TOP_DIR)/freertos/include \
		-I $(TOP_DIR)/freertos/portable/GCC/N200 \
		-I $(TOP_DIR)/Application


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LD_FILE = GD32VF103_Firmware_Library/RISCV/env_Eclipse/GD32VF103xB.lds
LDSCRIPT = $(PROJECTBASE)/$(LD_FILE)

# libraries
LIBS = -lm 
LIBDIR =
LDFLAGS = $(MCU) -nostartfiles -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(OBJ_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(OBJ_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

$(OBJ_DIR)/%.o: %.c Makefile | $(OBJ_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(OBJ_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(OBJ_DIR)/%.o: %.S Makefile | $(OBJ_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

ifeq ($(OBJ_DIR), $(wildcard $(OBJ_DIR)))
else
$(OBJ_DIR):$(BUILD_DIR)
	mkdir $@
endif

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# use gdb debug
#######################################
debug:
	$(GDB) -x $(BUILD_DIR)/../gdb.sh

#######################################
# dependencies
#######################################
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
