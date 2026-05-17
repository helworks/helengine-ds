DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
LIBNDS ?= $(DEVKITPRO)/libnds
HELENGINE_CORE_CPP_ROOT ?=
HELENGINE_DS_NITROFS_ROOT ?=

ifneq ($(strip $(HELENGINE_DS_NITROFS_ROOT)),)
NITRO_FILES := $(abspath $(HELENGINE_DS_NITROFS_ROOT))
$(if $(wildcard $(NITRO_FILES)),,$(error HELENGINE_DS_NITROFS_ROOT '$(HELENGINE_DS_NITROFS_ROOT)' does not exist))
endif

include $(DEVKITARM)/ds_rules

TARGET := helengine_ds
BUILD := build
SOURCES := \
	src \
	src/platform/ds
INCLUDES := \
	src
GENERATED_CORE_SOURCE_DIRS :=
GENERATED_CORE_CPPFILES :=
GENERATED_CORE_TRANSLATION_UNIT :=

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
$(if $(wildcard $(HELENGINE_CORE_CPP_ROOT)/helcpp_config.hpp),,$(error HELENGINE_CORE_CPP_ROOT '$(HELENGINE_CORE_CPP_ROOT)' does not contain helcpp_config.hpp))
ifneq ($(wildcard $(HELENGINE_CORE_CPP_ROOT)/helengine_core_amalgamated.cpp),)
GENERATED_CORE_TRANSLATION_UNIT := helengine_core_amalgamated.cpp
else ifneq ($(wildcard $(HELENGINE_CORE_CPP_ROOT)/helengine_core_unity.cpp),)
GENERATED_CORE_TRANSLATION_UNIT := helengine_core_unity.cpp
else
$(error HELENGINE_CORE_CPP_ROOT '$(HELENGINE_CORE_CPP_ROOT)' does not contain helengine_core_amalgamated.cpp or helengine_core_unity.cpp)
endif
$(if $(wildcard $(HELENGINE_CORE_CPP_ROOT)/runtime/runtime_startup_manifest.cpp),,$(error HELENGINE_CORE_CPP_ROOT '$(HELENGINE_CORE_CPP_ROOT)' does not contain runtime/runtime_startup_manifest.cpp))
CFLAGS += -DHELENGINE_NINTENDO_DS_HAS_GENERATED_CORE=1 -I$(HELENGINE_CORE_CPP_ROOT)
GENERATED_CORE_SOURCE_DIRS := $(HELENGINE_CORE_CPP_ROOT) $(HELENGINE_CORE_CPP_ROOT)/runtime
GENERATED_CORE_CPPFILES := $(GENERATED_CORE_TRANSLATION_UNIT) runtime_startup_manifest.cpp
ifneq ($(wildcard $(HELENGINE_CORE_CPP_ROOT)/runtime/runtime_scene_catalog_manifest.cpp),)
GENERATED_CORE_CPPFILES += runtime_scene_catalog_manifest.cpp
endif
ifneq ($(wildcard $(HELENGINE_CORE_CPP_ROOT)/runtime/runtime_code_module_manifest.cpp),)
GENERATED_CORE_CPPFILES += runtime_code_module_manifest.cpp
endif
endif

CFLAGS += $(INCLUDE) -DARM9
CXXFLAGS := $(CFLAGS) -std=gnu++17
ifeq ($(strip $(HELENGINE_CORE_CPP_ROOT)),)
CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-exceptions
endif
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(TARGET).map -Wl,--gc-sections

LIBS := -lnds9 -lfat
LIBDIRS := $(LIBNDS)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(BUILD)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) $(GENERATED_CORE_SOURCE_DIRS)
export DEPSDIR := $(CURDIR)/$(BUILD)
ifneq ($(strip $(HELENGINE_DS_NITROFS_ROOT)),)
export NITRO_FILES := $(NITRO_FILES)
endif

CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
CPPFILES += $(GENERATED_CORE_CPPFILES)
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

NintendoDsRenderManager2D.o: CXXFLAGS := $(filter-out -mthumb,$(CXXFLAGS)) -marm

$(OUTPUT).nds: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)

%.o: %.bin
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif
