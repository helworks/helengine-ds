DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
LIBNDS ?= $(DEVKITPRO)/libnds
HELENGINE_CORE_CPP_ROOT ?=

include $(DEVKITARM)/ds_rules

TARGET := helengine_ds
BUILD := build
SOURCES := \
	src \
	src/platform/ds
INCLUDES := \
	src

ARCH := -march=armv5te -mtune=arm946e-s -mthumb

CFLAGS := \
	-g \
	-O2 \
	-Wall \
	-Wextra \
	-ffunction-sections \
	-fdata-sections \
	$(ARCH)

ifeq ($(strip $(HELENGINE_CORE_CPP_ROOT)),)
CFLAGS += -DHELENGINE_NINTENDO_DS_HAS_GENERATED_CORE=0
else
CFLAGS += -DHELENGINE_NINTENDO_DS_HAS_GENERATED_CORE=1 -I$(HELENGINE_CORE_CPP_ROOT)
endif

CFLAGS += $(INCLUDE) -DARM9
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-rtti -fno-exceptions
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(TARGET).map -Wl,--gc-sections

LIBS := -lnds9
LIBDIRS := $(LIBNDS)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(BUILD)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.bin)))

ifeq ($(strip $(CPPFILES)),)
export LD := $(CC)
else
export LD := $(CXX)
endif

export OFILES := $(BINFILES:.bin=.o) \
	$(CPPFILES:.cpp=.o) \
	$(CFILES:.c=.o) \
	$(SFILES:.s=.o)

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
	$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
	-I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all clean

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).ds.gba

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).nds: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)

%.o: %.bin
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif
