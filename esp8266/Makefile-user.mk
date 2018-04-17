## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

## Add your source directories here separated by space
MODULES = app lib/core
EXTRA_INCDIR = include lib/core

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
# ESP_HOME = /opt/esp-open-sdk

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

## MacOS / Linux
# SMING_HOME = /opt/sming/Sming

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
# COM_PORT = COM3

## MacOS / Linux:
# COM_PORT = /dev/tty.usbserial

## Com port speed
# COM_SPEED	= 115200

## Configure flash parameters (for ESP12-E and other new boards):
# SPI_MODE = dio

## SPIFFS options
#DISABLE_SPIFFS = 1
SPIFF_FILES = web/build

#### overridable rBoot options ####
## use rboot build mode
RBOOT_ENABLED ?= 1
## enable big flash support (for multiple roms, each in separate 1mb block of flash)
RBOOT_BIG_FLASH ?= 1
RBOOT_RTC_ENABLED = 1
RBOOT_GPIO_ENABLED = 1 # Boot via the GPIO will run the factory default ROM.
RBOOT_GPIO_SKIP_ENABLED = 0 # `RBOOT_GPIO_SKIP_ENABLED` and `RBOOT_GPIO_ENABLED` cannot be used at the same time.

RBOOT_ROM1_ADDR=0x202000 # 3rd MB
RBOOT_ROM2_ADDR=0x302000 # 4th MB

## two rom mode (where two roms sit in the same 1mb block of flash)
#RBOOT_TWO_ROMS  ?= 1
## size of the flash chip
SPI_SIZE        ?= 4M
## output file for first rom (.bin will be appended)
#RBOOT_ROM_0     ?= rom0
## input linker file for first rom
#RBOOT_LD_0      ?= rom0.ld
## these next options only needed when using two rom mode
#RBOOT_ROM_1     ?= rom1
#RBOOT_LD_1      ?= rom1.ld
SPIFF_START_OFFSET ?= $(RBOOT_SPIFFS_0)
## size of the spiffs to create
SPIFF_SIZE      ?= 512000
## option to completely disable spiffs
#DISABLE_SPIFFS  = 1
## flash offsets for spiffs, set if using two rom mode or not on a 4mb flash
## (spiffs location defaults to the mb after the rom slot on 4mb flash)
#RBOOT_SPIFFS_0  ?= 0x100000
#RBOOT_SPIFFS_1  ?= 0x300000
## esptool2 path
#ESPTOOL2        ?= esptool2

# [Memory Map]

# TSB Settings (Update them only here according to the updated flash map)
TSB_DEFAULT_MARK_ADDR= 0x300000
TSB_CURRENT_MARK_ADDR= 0x301000
TSB_DEFAULT_ROM_ADDR = 0x3f0000
TSB_CURRENT_ROM_ADDR = 0x3f4000

# Device Info Settings
MANUFACTURER_DATA_ADDR = 0x00000ea0
DEVICE_DATA_ADDR=0x200000

# Application data address
APP_DATA_ADDR=0x1fe000

# Crash Data address
CRASH_DATA_ADDR=0x1fd000

# [Sming Compile Settings] #

# SSL settings
ENABLE_SSL=1

# Custom Heap Settings
ENABLE_CUSTOM_HEAP ?= 0 

# Use custom LWIP
ENABLE_CUSTOM_LWIP=1
ENABLE_ESPCONN=1

# Preserve heap by disabling command executor
ENABLE_CMD_EXECUTOR=0

export ENABLE_SSL
export ENABLE_CUSTOM_LWIP
export ENABLE_ESPCONN
export ENABLE_CMD_EXECUTOR

# Should be enabled after the first phase

#Modes
RELEASE ?= 0
TEST_RELEASE ?=1

# Deployment Settings
REMOTE_USER=server
REMOTE_HOST=attachix.com
REMOTE_FOLDER=/home/server/dev/attachix-web/public/update
REMOTE_OPTIONS="-P 10022"

# Debug - hide completely debug messages
#DEBUG=1 
# Do not show any visual indication
#STEALTH_MODE=1 
# Allow test commands to be used
# TEST_MODE=1

USER_CFLAGS +=-mforce-l32

ifeq ($(RELEASE), 1)
	USER_CFLAGS +=-DRELEASE=1 -DSTEALTH_MODE=1
else ifeq ($(TEST_RELEASE), 1)
	USER_CFLAGS += -DTEST_MODE=1
else 
	DEBUG_VERBOSE_LEVEL=3
	export DEBUG_VERBOSE_LEVEL
	USER_CFLAGS +=-DDEBUG=1 -DTEST_MODE=1 -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL)
endif

# Experimental Features
ENABLE_EXPERIMENTAL ?= 1

ENABLE_JSVM ?= 0 # allow JerryScript VM if experimental mode is set
ENABLE_WEBSERVER ?=0 # allow web server to be running...
ENABLE_OTA  ?= 1 # allow OverTheAir updates
ENABLE_OTA_TSB ?= 1 # Allow updates of CPU1 over the air using TSB bootloader
ENABLE_OTA_DEFAULT_ROM ?= 0

USER_CFLAGS +=-DTSB_DEFAULT_MARK_ADDR=$(TSB_DEFAULT_MARK_ADDR) -DTSB_CURRENT_MARK_ADDR=$(TSB_CURRENT_MARK_ADDR) \
			  -DTSB_DEFAULT_ROM_ADDR=$(TSB_DEFAULT_ROM_ADDR) -DTSB_CURRENT_ROM_ADDR=$(TSB_CURRENT_ROM_ADDR) \
			  -DMANUFACTURER_DATA_ADDR=$(MANUFACTURER_DATA_ADDR) -DDEVICE_DATA_ADDR=$(DEVICE_DATA_ADDR) \
			  -DAPP_DATA_ADDR=$(APP_DATA_ADDR) -DCRASH_DATA_ADDR=$(CRASH_DATA_ADDR)

ifeq ($(ENABLE_OTA_DEFAULT_ROM), 1)
	USER_CFLAGS +=-DRBOOT_GPIO_ENABLED=1 -DBOOT_GPIO_ENABLED=1 -DRBOOT_ROM1_ADDR=$(RBOOT_ROM1_ADDR) -DRBOOT_ROM2_ADDR=$(RBOOT_ROM2_ADDR)
endif

ifeq ($(ENABLE_EXPERIMENTAL), 1)
	USER_CFLAGS +=-DENABLE_JSVM=$(ENABLE_JSVM) -DENABLE_OTA=$(ENABLE_OTA) -DENABLE_OTA_TSB=$(ENABLE_OTA_TSB) \
				  -DENABLE_OTA_DEFAULT_ROM=$(ENABLE_OTA_DEFAULT_ROM) \
				  -DENABLE_WEBSERVER=$(ENABLE_WEBSERVER)
endif

ifeq ($(RELEASE), 1)
	# this requires also recompilation of Sming with the SMING_RELEASE flag.
	export SMING_RELEASE=1
endif

ifeq ($(ENABLE_JSVM), 1)
	MODULES += lib/jsvm
	EXTRA_INCDIR += lib/jerryscript lib/jsvm
	EXTRA_LIBS = jerrycore
endif

export ENABLE_JSVM
export ENABLE_OTA
export ENABLE_OTA_TSB
export ENABLE_OTA_DEFAULT_ROM



