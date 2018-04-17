-include Makefile.local

TOOLCHAIN_PREFIX := xtensa-lx106-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
AR := $(TOOLCHAIN_PREFIX)ar
LD := $(TOOLCHAIN_PREFIX)gcc
AR := $(TOOLCHAIN_PREFIX)ar
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy


XTENSA_LIBS ?= $(shell $(CC) -print-sysroot)
ifeq ($(XTENSA_LIBS),)
	XTENSA_LIBS = $(ESP_HOME)/xtensa-lx106-elf/
endif


BUILD_BASE	= out/build

COMPACT_PROFILE ?= 1

SRC_DIR := $(shell find jerry-core/ -type d)

SRC			:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*))
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(SRC_DIR))
OBJ_FILES	:= $(patsubst %.c,%.o,$(SRC))

INCDIR		:= $(addprefix -I,$(SRC_DIR))

LDFLAGS  += -L$(XTENSA_LIBS)/lib \
			-L$(XTENSA_LIBS)/arch/lib \

CFLAGS+=-std=c99 -D__TARGET_ESP8266

CFLAGS += -Wall -Os -g -O2 -Wpointer-arith -Wno-implicit-function-declaration -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mno-text-section-literals  -D__ets__ -DICACHE_FLASH

# CFLAGS += -fno-builtin -fno-stack-protector -g -gdwarf-4 -Wall -Werror=all -Wextra -Werror=extra -Wformat-nonliteral -Werror=format-nonliteral -Winit-self -Werror=init-self -Wconversion -Werror=conversion -Wsign-conversion -Werror=sign-conversion -Wformat-security -Werror=format-security -Wmissing-declarations -Werror=missing-declarations -Wno-stack-protector -Wno-attributes -Wlogical-op -Werror=logical-op  -D__attr_always_inline___= -Wl,-EL -fno-inline-functions -ffunction-sections -fdata-sections -mlongcalls -mtext-section-literals -mno-serialize-volatile -pedantic -Os
ifeq ($(COMPACT_PROFILE), 1)
   CFLAGS += -DCONFIG_DISABLE_BUILTINS \
		     -DCONFIG_DISABLE_ES2015 \
		     -DCONFIG_DISABLE_UNICODE_CASE_CONVERSION
endif

CFLAGS += -DCONFIG_ECMA_NUMBER_TYPE=CONFIG_ECMA_NUMBER_FLOAT32 -DCONFIG_MEM_HEAP_AREA_SIZE=512 \
		  -DCONFIG_ECMA_LCACHE_DISABLE  \
		  -DJERRY_DISABLE_JS_PARSER -DJERRY_ENABLE_SNAPSHOT_EXEC -DJERRY_NDEBUG -D_BSD_SOURCE \
		  -D__TARGET_ESP8266 -D__attr_always_inline___=

MFORCE32 := $(shell $(CC) --help=target | grep mforce-l32)

ifneq ($(MFORCE32),)
    # Your compiler supports the -mforce-l32 flag which means that 
    # constants can be placed in ROM to free additional RAM
    CFLAGS += -DJERRY_CONST_DATA="__attribute__((aligned(4))) __attribute__((section(\".irom.text\")))" -mforce-l32 
endif

CFLAGS += $(INCDIR) $(SDK_INCDIR)

BIN_DIR := bin
JSCORE_AR := $(BIN_DIR)/libjerrycore.a

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

all: patchjmp $(JSCORE_AR) $(BUILD_DIR)
rebuild: clean all

$(BUILD_DIR):
	$(Q) mkdir -p $@

$(JSCORE_AR): | $(BIN_DIR)

$(JSCORE_AR): $(OBJ_FILES)
	for file in $(OBJ_FILES); do \
		$(OBJCOPY) \
		--rename-section .text=.irom0.text \
		--rename-section .literal=.irom0.literal \
		$$file; \
	done
	$(AR) cru $@ $^
	
patchjmp: $(JSCORE_AR)
	$(Q) $(AR) x ${XTENSA_TOOLS_ROOT}/../xtensa-lx106-elf/sysroot/lib/libcirom.a lib_a-setjmp.o
	$(Q) $(AR) r ${JSCORE_AR} lib_a-setjmp.o
	$(Q) rm lib_a-setjmp.o

$(BIN_DIR):
	$(Q) mkdir -p $(BIN_DIR)

clean:
	$(Q) rm -rf $(OBJ_FILES) $(JSCORE_AR) $(BUILD_DIR)


.PHONY: all clean
