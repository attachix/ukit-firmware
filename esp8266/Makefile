.DEFAULT_GOAL := build
#####################################################################
#### Please don't change this file. Use Makefile-user.mk instead ####
#####################################################################
# Including user Makefile.
# Should be used to set project-specific parameters
include ./Makefile-user.mk

# Important parameters check.
# We need to make sure SMING_HOME and ESP_HOME variables are set.
# You can use Makefile-user.mk in each project or use enviromental variables to set it globally.
 
ifndef SMING_HOME
$(error SMING_HOME is not set. Please configure it in Makefile-user.mk)
endif
ifndef ESP_HOME
$(error ESP_HOME is not set. Please configure it in Makefile-user.mk)
endif

CURRENT_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

GIT_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
GIT_BRANCH  := $(shell git rev-parse --abbrev-ref HEAD)
SMING_VERSION := $(shell git -C $(SMING_HOME) log -n 1 --pretty=format:"%h")

ifeq ($(ENABLE_OTA),1)
	FEATURES += OTA
endif
ifeq ($(ENABLE_JSVM),1)
	FEATURES += JSVM
endif
ifeq ($(ENABLE_WEBSERVER),1)
	FEATURES += WebServer
endif

DATE_VERSION :=$(shell date +'%Y.%m.%d.%H')

USER_CFLAGS +=-DFW_VERSION="\"$(DATE_VERSION)-$(GIT_VERSION):$(SMING_VERSION)@$(GIT_BRANCH):$(FEATURES)\""

ifeq ($(TEST_BUZZ), 1)
	USER_CFLAGS += -DTEST_BUZZ=1
endif

ifeq ($(TEST_BLINK), 1)
	USER_CFLAGS += -DTEST_BLINK=1
endif

ifeq ($(TEST_AUTO_OTA), 1)
	USER_CFLAGS += -DTEST_AUTO_OTA=1
endif

# Include main Sming Makefile
ifeq ($(RBOOT_ENABLED), 1)
include $(SMING_HOME)/Makefile-rboot.mk
else
include $(SMING_HOME)/Makefile-project.mk
endif

AS		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AS_SRC	:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.s))
AS_OBJ	:= $(patsubst %.s,$(BUILD_BASE)/%.o,$(AS_SRC))
OBJ := $(OBJ) $(AS_OBJ)

JSVM_AR = $(BUILD_BASE)/libjerrycore.a

$(BUILD_BASE):
	$(Q) mkdir -p $@

### Error Rerporting Target ###

$(AS_OBJ): $(AS_SRC)
	$(Q) mkdir -p $(dir $@)
	$(Q) $(AS) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS) -c $< -o $@
	$(Q) $(AR) r $(APP_AR) $@

### JSVM Targets ###

jsvm-clean:
	$(Q) $(MAKE) -C lib/jerryscript/ BIN_DIR="$(CURRENT_DIR)/$(BUILD_BASE)" clean
	$(Q) rm -rf $(JSVM_AR)
	
$(JSVM_AR): $(BUILD_BASE)
ifeq ($(ENABLE_JSVM),1)
	$(Q) $(MAKE) -C lib/jerryscript/ BIN_DIR="$(CURRENT_DIR)/$(BUILD_BASE)"
else
	$(vecho) "(!) JSVM is in experimental phase. Use ENABLE_JSVM=1 to turn it on."
endif

### Deployment And Release Targets ###
RELEASE_DIR=out/releases

ifneq ($(DISABLE_SPIFFS), 1)
	EXTRA_OPTS += --spiffs=$(CURRENT_DIR)/$(RELEASE_DIR)/spiff_rom.bin
endif

ifeq ($(FLASH_INIT_DATA), 1) 
	EXTRA_OPTS += --init=$(SDK_BASE)/bin/esp_init_data_default.bin
endif

deploy: 
	$(Q) $(MAKE) RELEASE=0 TEST_RELEASE=1 DISABLE_SPIFFS=1
	$(Q) cp out/firmware/rom0.bin firmware/rom0-9600.bin
	$(Q) scp $(REMOTE_OPTIONS) firmware/rom0-9600.bin ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_FOLDER}/rom0.bin
	$(Q) $(MAKE) clean
	$(Q) $(MAKE) RELEASE=0 TEST_RELEASE=0 DISABLE_SPIFFS=1
	$(Q) cp out/firmware/rom0.bin firmware/rom0-9600-debug.bin
	$(Q) scp $(REMOTE_OPTIONS) firmware/rom0-9600-debug.bin ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_FOLDER}/rom0-debug.bin

firmware/rom1.bin: ../mcu1/src/build/main.hex
	$(Q) hex2bin.py $< $@

firmware/rom1-debug.bin: ../mcu1/src/build/main.hex
	$(Q) hex2bin.py $< $@

deploy1:
	$(Q) scp $(REMOTE_OPTIONS) firmware/rom1.bin ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_FOLDER}/rom1.bin
	$(Q) scp $(REMOTE_OPTIONS) firmware/rom1-debug.bin ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_FOLDER}/rom1-debug.bin
	
release-clean:
	$(Q) rm -rf $(RELEASE_DIR)

$(RELEASE_DIR): 
	$(Q) mkdir -p $(RELEASE_DIR)
	
$(RELEASE_DIR)/rom0.bin: $(RELEASE_DIR)
	$(Q) $(MAKE) clean
	$(Q) $(MAKE)
	$(Q) cp out/firmware/spiff_rom.bin $(RELEASE_DIR)/spiff_rom.bin
	$(Q) cp out/firmware/rom0.bin $(RELEASE_DIR)/rom0.bin

$(RELEASE_DIR)/rom0-default.bin: $(RELEASE_DIR)
	$(Q) $(MAKE) clean
	$(Q) $(MAKE) DISABLE_SPIFFS=1 ENABLE_OTA_DEFAULT_ROM=1 V=1
	$(Q) cp out/firmware/rboot.bin $(RELEASE_DIR)/rboot.bin
	$(Q) cp out/firmware/rom0.bin $(RELEASE_DIR)/rom0-default.bin

release: $(RELEASE_DIR) $(RELEASE_DIR)/rom0-default.bin $(RELEASE_DIR)/rom0.bin firmware/rom1.bin
	python ../tools/flasher.py --addr-rom0=0x02000 --addr-romD=$(RBOOT_ROM2_ADDR) \
				--addr-spiffs=$(SPIFF_START_OFFSET) --addr-device=$(DEVICE_DATA_ADDR) \
				--addr-romX=$(TSB_DEFAULT_ROM_ADDR) --addr-mark=$(TSB_DEFAULT_MARK_ADDR) \
				--config=$(SDK_BASE)/bin/blank.bin \
				--rom0=$(CURRENT_DIR)/$(RELEASE_DIR)/rom0.bin --romD=$(CURRENT_DIR)/$(RELEASE_DIR)/rom0-default.bin \
			    --romX=firmware/rom1.bin --boot=$(CURRENT_DIR)/$(RELEASE_DIR)/rboot.bin $(EXTRA_OPTS)
	
rebuild: clean jsvm-clean  build
	$(vecho) "Rebuild done."
	
build: $(AS_OBJ) $(JSVM_AR) all 
	$(vecho) "Build finished."
	
pre-update: out/firmware/rom0.bin
	$(Q) python ../tools/romupdater.py -a 0x02000 -n $< -o $<.last
	 	
update:  pre-update
	$(Q) cp -f out/firmware/rom0.bin out/firmware/rom0.bin.last 
	

firmware:
	$(Q) mkdir -p firmware

test-ota-mcu1: $(RELEASE_DIR) $(BUILD_BASE) firmware
	$(vecho) "Flash AVR-APP with TEST_BUZZ=1"
	$(vecho) "Compile AVR-APP with TEST_BLINK=1 TEST_AUTO_OTA=1"
	$(Q) touch ../mcu1/src/main.asm
	$(Q) make -C ../mcu1/src/ TEST_BLINK=1 TEST_AUTO_OTA=1
	$(Q) make -C ../mcu1/src/ ota-app
	$(vecho) "Compile AVR-APP with TEST_BLINK=1 and TEST_BUZZ=1"
	$(Q) touch ../mcu1/src/main.asm 
	$(Q) make -C ../mcu1/src/ build/main.bin TEST_BLINK=1 TEST_BUZZ=1
	$(Q) cp ../mcu1/src/build/main.bin firmware/rom1.bin
	$(vecho) "compile AVR-APP with TEST_BUZZ=1" 
	$(Q) touch ../mcu1/src/main.asm 
	$(Q) make -C ../mcu1/src/ build/main.bin TEST_BUZZ=1
	$(Q) scp $(REMOTE_OPTIONS) ../mcu1/src/build/main.bin ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_FOLDER}/rom1.bin 
	
test-ota-mcu2: $(RELEASE_DIR) $(BUILD_BASE) firmware
	$(vecho) "Compile ESP-APP with TEST_BUZZ and send it to remote server"
	$(Q) make clean
	$(Q) make TEST_BUZZ=1
	$(Q) scp $(REMOTE_OPTIONS) out/firmware/rom0.bin ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_FOLDER}/rom0.bin
	$(vecho) "Compile ESP-APP with TEST_BLINK=1 and TEST_AUTO_OTA=1 and save it to releases"
	$(Q) make clean
	$(Q) make TEST_BLINK=1 TEST_AUTO_OTA=1
	$(Q) cp out/firmware/rom0.bin $(RELEASE_DIR)/rom0.bin
	$(vecho) "Compile ESP-APP with TEST_BLINK and TEST_BUZZ and save it to releases as factory default"
	$(Q) $(MAKE) clean
	$(Q) $(MAKE) DISABLE_SPIFFS=1 ENABLE_OTA_DEFAULT_ROM=1 TEST_BUZZ=1 TEST_BLINK=1
	$(Q) cp out/firmware/rboot.bin $(RELEASE_DIR)/rboot.bin
	$(Q) cp out/firmware/rom0.bin $(RELEASE_DIR)/rom0-default.bin
	$(vecho) "Prepare MCU2 for flashing. Press enter when ready"; read line
	$(Q) $(MAKE) release
	
test-ota: test-ota-mcu1 test-ota-mcu2

web-pack:
	$(Q) rm -rf web/build
	$(Q) gulp
	$(Q) date +'%a, %d %b %Y %H:%M:%S GMT' -u > web/build/.lastModified

snap-js: web-pack
	$(Q) nodejs web/dev/snapshot-compiler.js -o /tmp/main.js.snap web/dev/main.js
	$(Q) python tools/txt2bin.py /tmp/main.js.snap > web/build/main.js.snap
        
web-upload: snap-js spiff_update
	$(ESPTOOL) -p $(COM_PORT) -b $(COM_SPEED_ESPTOOL) write_flash $(flashimageoptions) $(SPIFF_START_OFFSET) $(SPIFF_BIN_OUT)

