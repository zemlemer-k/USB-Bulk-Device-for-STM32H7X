#Project name
PROJECT = bulk_usb

# OS specific commands
RM = rm -rf
MKDIR = mkdir -p

# Build directories
OBJDIR = $(CURDIR)/obj
BINDIR = $(CURDIR)/bin

#Targets
BIN = $(BINDIR)/$(PROJECT).bin
HEX = $(BINDIR)/$(PROJECT).hex
ELF = $(BINDIR)/$(PROJECT).elf
LST = $(OBJDIR)/$(PROJECT).lst
MAP = $(OBJDIR)/$(PROJECT).map

# Toolcahin setup
TOOLCHAIN = /opt/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi

TOOLCHAIN_BIN = $(TOOLCHAIN)/bin/
TOOLCHAIN_LIB = $(TOOLCHAIN)/arm-none-eabi/lib/thumb/v7e-m+fp/hard 
TOOLCHAIN_INC = $(TOOLCHAIN)/arm-none-eabi/include




CXX = $(TOOLCHAIN_BIN)arm-none-eabi-g++
CC = $(TOOLCHAIN_BIN)arm-none-eabi-gcc
AS = $(TOOLCHAIN_BIN)arm-none-eabi-as
LD = $(TOOLCHAIN_BIN)arm-none-eabi-ld
CP = $(TOOLCHAIN_BIN)arm-none-eabi-objcopy
OD = $(TOOLCHAIN_BIN)arm-none-eabi-objdump
SZ = $(TOOLCHAIN_BIN)arm-none-eabi-size

# Flash tools
FT = st-flash
#dfu tools
# DT = /usr/bin/dfu-util


#------------------------------------------------------------------------------
# Global compiler, assembler and linker flags
#------------------------------------------------------------------------------
# cpu
CPU = -mcpu=cortex-m7
FPU = -mfpu=fpv5-d16
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

#OPT := -Os
OPT ?= -Os
#OPT += -g

MCUFLAGS := $(MCU)

ASFLAGS := $(MCUFLAGS)
ASFLAGS += -x assembler-with-cpp -Wall -c
ASFLAGS += --specs=nano.specs

CFLAGS := $(MCUFLAGS)
CFLAGS += $(OPT)
#CFLAGS += --specs=nosys.specs
CFLAGS += --specs=nano.specs
#CFLAGS += --specs=rdimon.specs
#CFLAGS += -Wl,--start-group -lgcc -lc -lm -lrdimon -Wl,--end-group


CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall -pedantic
CFLAGS += -Wl,--gc-sections
CFLAGS += -fno-strict-aliasing
CFLAGS += -std=c99
CFLAGS += -ffast-math
CFLAGS += -fmax-errors=3

LDFLAGS := $(MCUFLAGS)
LDFLAGS += -Wl,-Map=$(MAP)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--print-memory-usage
LDFLAGS += -fno-exceptions -fno-rtti
LDFLAGS += --specs=nano.specs
LDFLAGS += -L$(TOOLCHAIN_LIB)
LDFLAGS += -Wl,--start-group -lgcc -lc -lm -Wl,--end-group

#------------------------------------------------------------------------------
# Project definitions
#------------------------------------------------------------------------------
DEFS :=
DEFS += USE_HAL_DRIVER
DEFS += STM32H750xx
DEFS += HSE_VALUE=25000000
DEFS += DATA_IN_D2_SRAM
DEFS += VER_MAJOR=0
DEFS += VER_MINOR=1

CFLAGS +=$(foreach i, $(DEFS), -D$(i))

#------------------------------------------------------------------------------
# Project include directories
#------------------------------------------------------------------------------
# include files directories
INCDIRS := $(TOOLCHAIN_INC)
INCDIRS += $(CURDIR)/Core/Inc
INCDIRS += $(CURDIR)/Project/Inc
INCDIRS += $(CURDIR)/STM32H7xx_HAL_Driver/Inc
INCDIRS += $(CURDIR)/STM32_USB_Device_Library/Inc

CFLAGS +=$(foreach i, $(INCDIRS), -I$(i))

#------------------------------------------------------------------------------
# Project source directories
#------------------------------------------------------------------------------
CORE_DIR := $(CURDIR)/Core/Src
HAL_DIR := $(CURDIR)/STM32H7xx_HAL_Driver/Src
USB_DIR := $(CURDIR)/STM32_USB_Device_Library/Src

#LWIP_CORE_DIR := $(CURDIR)/LwIP/src/core
#LWIP_CORE_IP4_DIR := $(CURDIR)/LwIP/src/core/ipv4
#LWIP_NETIF_DIR :=  $(CURDIR)/LwIP/src/netif

#LIB_DSP_TRN_DIR := $(CURDIR)/DSP/Source/TransformFunctions
#LIB_DSP_COM_DIR := $(CURDIR)/DSP/Source/CommonTables
#LIB_DSP_CMF_DIR := $(CURDIR)/DSP/Source/ComplexMathFunctions
#LIB_DSP_FM_DIR := $(CURDIR)/DSP/Source/FastMathFunctions


PROJECT_DIR := $(CURDIR)/Project/Src

VPATH := $(CORE_DIR)
VPATH += $(HAL_DIR)
VPATH += $(USB_DIR)
#VPATH += $(LWIP_CORE_DIR)
#VPATH += $(LWIP_CORE_IP4_DIR)
#VPATH += $(LWIP_NETIF_DIR)

#VPATH += $(LIB_DSP_TRN_DIR)
#VPATH += $(LIB_DSP_COM_DIR)
#VPATH += $(LIB_DSP_CMF_DIR)
#VPATH += $(LIB_DSP_FM_DIR)

VPATH += $(PROJECT_DIR)



#------------------------------------------------------------------------------
# Linker script
#------------------------------------------------------------------------------
LDSCRIPT := $(CURDIR)/Core/STM32H750XX_FLASH.ld
LDFLAGS += -T$(LDSCRIPT)

#------------------------------------------------------------------------------
#Startup script
#------------------------------------------------------------------------------
SRC := $(CORE_DIR)/startup_stm32h750xx.s

#------------------------------------------------------------------------------
# Core files
#------------------------------------------------------------------------------
SRC += $(CORE_DIR)/system_stm32h7xx.c
SRC += $(CORE_DIR)/stm32h7xx_it.c

SRC += $(CORE_DIR)/syscalls.c
#SRC += $(CORE_DIR)/sysmem.c

#------------------------------------------------------------------------------
# STM32 USB library
#------------------------------------------------------------------------------
SRC += $(USB_DIR)/usbd_core.c
SRC += $(USB_DIR)/usbd_bd.c
SRC += $(USB_DIR)/usbd_ioreq.c
SRC += $(USB_DIR)/usbd_ctlreq.c
SRC += $(USB_DIR)/usbd_desc.c
SRC += $(USB_DIR)/usbd_init.c
SRC += $(USB_DIR)/usbd_conf.c

#------------------------------------------------------------------------------
# HAL Device library
#------------------------------------------------------------------------------
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_adc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_adc_ex.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_cec.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_comp.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_cordic.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_cortex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_crc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_crc_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_cryp.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_cryp_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dac.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dac_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dcmi.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dfsdm.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dfsdm_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dma2d.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dma.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dma_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dsi.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_dts.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_eth.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_eth_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_exti.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_fdcan.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_flash.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_flash_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_fmac.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_gfxmmu.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_gpio.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_hash.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_hash_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_hcd.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_hrtim.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_hsem.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_i2c.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_i2c_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_i2s.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_i2s_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_irda.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_iwdg.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_jpeg.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_lptim.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_ltdc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_ltdc_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_mdios.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_mdma.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_mmc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_mmc_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_msp_template.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_nand.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_nor.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_opamp.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_opamp_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_ospi.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_otfdec.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_pcd.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_pcd_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_pssi.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_pwr.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_pwr_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_qspi.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_ramecc.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_rcc.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_rcc_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_rng.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_rng_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_rtc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_rtc_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_sai.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_sai_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_sd.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_sd_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_sdram.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_smartcard.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_smartcard_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_smbus.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_smbus_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_spdifrx.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_spi.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_spi_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_sram.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_swpmi.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_tim.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_timebase_rtc_alarm_template.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_timebase_rtc_wakeup_template.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_timebase_tim_template.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_tim_ex.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_uart.c
SRC += $(LIB_HW_DIR)/stm32h7xx_hal_uart_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_usart.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_usart_ex.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_hal_wwdg.c


# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_adc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_bdma.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_comp.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_cordic.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_crc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_crs.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_dac.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_delayblock.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_dma2d.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_dma.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_exti.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_fmac.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_fmc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_gpio.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_hrtim.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_i2c.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_lptim.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_lpuart.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_mdma.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_opamp.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_pwr.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_rcc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_rng.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_rtc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_sdmmc.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_spi.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_swpmi.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_tim.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_usart.c
SRC += $(LIB_HW_DIR)/stm32h7xx_ll_usb.c
# SRC += $(LIB_HW_DIR)/stm32h7xx_ll_utils.c

#------------------------------------------------------------------------------
# Project files
#------------------------------------------------------------------------------
SRC += $(PROJECT_DIR)/main.c
SRC += $(PROJECT_DIR)/lib_terminal.c
SRC += $(PROJECT_DIR)/bd_leds.c
SRC += $(PROJECT_DIR)/bd_uart.c
SRC += $(PROJECT_DIR)/bd_usbexch.c

#------------------------------------------------------------------------------
# Object files
#------------------------------------------------------------------------------
OBJS := $(notdir $(SRC))
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.s=.o)

OBJS := $(patsubst %, $(OBJDIR)/%, $(OBJS))

#------------------------------------------------------------------------------
# Build process
#------------------------------------------------------------------------------
all: $(ELF) $(BIN) $(HEX) $(LST) RESULT

RESULT: $(ELF)
	@echo -------------------------------------------------------- BUILD RESULT --------------------------------------------------------
	$(SZ) --format=berkeley $(ELF)
	@echo ------------------------------------------------------------------------------------------------------------------------------

$(LST): $(ELF)
	$(OD) -h -S $(ELF) > $(LST)

$(BIN): $(ELF)
	$(CP) -O binary $(ELF) $@

$(HEX): $(ELF)
	$(CP) -O ihex $(ELF) $@

$(ELF): $(OBJDIR) $(BINDIR) $(OBJS)
	@echo --== Linking $(ELF) ==--
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

$(BINDIR):
	$(MKDIR) $(BINDIR)

$(OBJDIR)/%.o : %.c
	@echo --== Compiling $< ==--
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : %.s
	@echo --== Assembling $< ==--
	$(CC) -c $(ASFLAGS) $< -o $@


.PHONY: clean rebuild load cppcheck

clean:
	$(RM) $(OBJS) $(LST) $(MAP) $(ELF) $(BIN) $(HEX)
	$(RM) -d $(OBJDIR)

load:
	$(MAKE)
#	$(DT)  --device 0483:df11 --alt 0 --dfuse-address 0x08000000 --download $(BIN)
	$(FT)  --reset write $(BIN) 0x08000000

rebuild:
	$(MAKE) clean
	$(MAKE) -j4

cppcheck:
	cppcheck --enable=all --language=c --std=c99 --inconclusive \
	--suppress=missingInclude $(PROJECT_DIR) -i $(INCDIRS)

