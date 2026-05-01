# Helengine Nintendo DS Bootstrap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first Docker-only Nintendo DS host that compiles to `build/helengine_ds.nds` and boots in melonDS or DeSmuME with a green top screen and cyan bottom screen.

**Architecture:** The DS project stays intentionally small: one devkitPro-based Docker image, one `Makefile`, one tiny `main.cpp`, and one `NintendoDsBootHost` class that owns DS video initialization and the infinite verification frame loop. The build exposes a reserved generated-core root variable now, but phase 1 does not compile generated code or add fake engine integration.

**Tech Stack:** Docker, devkitPro, devkitARM, libnds, `ds_rules`, C++17, GNU Make.

---

## File Structure

- `Dockerfile`
  Responsible for provisioning the Linux-based devkitPro Nintendo DS build environment and dropping the container into `/workspace`.
- `Makefile`
  Responsible for compiling the Nintendo DS sources, linking the ARM9 executable, packaging the `.nds`, and exposing the future generated-core root variable.
- `README.md`
  Responsible for documenting the Docker build flow, the output artifact, and the melonDS/DeSmuME verification steps.
- `src/main.cpp`
  Responsible only for constructing `NintendoDsBootHost`, calling `Run()`, and returning the exit code.
- `src/platform/ds/NintendoDsBootHost.hpp`
  Responsible for declaring the thin Nintendo DS bootstrap boundary and its native state.
- `src/platform/ds/NintendoDsBootHost.cpp`
  Responsible for DS display initialization, 16-bit bitmap background setup, framebuffer writes, and the infinite verification frame loop.

### Task 1: Docker build environment and toolchain inspection

**Files:**
- Create: `Dockerfile`
- Create: `README.md`

- [ ] **Step 1: Create the Docker image definition**

```dockerfile
FROM devkitpro/devkitarm:latest

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=${DEVKITPRO}/devkitARM
ENV LIBNDS=${DEVKITPRO}/libnds
ENV PATH=${DEVKITARM}/bin:${DEVKITPRO}/tools/bin:${PATH}

RUN dkp-pacman -Syu --noconfirm \
    && dkp-pacman -S --noconfirm nds-dev

WORKDIR /workspace
CMD ["/bin/bash"]
```

- [ ] **Step 2: Build the image**

Run:

```bash
rtk docker build -t helengine-ds .
```

Expected:

```text
The build completes successfully and produces a local image tagged helengine-ds.
```

- [ ] **Step 3: Inspect the DS toolchain layout inside the container**

Run:

```bash
rtk docker run --rm helengine-ds sh -lc 'find /opt/devkitpro \( -name ds_rules -o -name ds_arm9.specs -o -name libnds9.a \) | sort'
```

Expected:

```text
The output includes the current ds_rules path, a ds_arm9.specs path, and the libnds9 static library path.
```

- [ ] **Step 4: Create the initial README focused on the first milestone**

````markdown
# Helengine Nintendo DS Host

This repository contains the native Nintendo DS host scaffold for Helengine.

## Current milestone

- Docker-only build using devkitPro Nintendo DS tooling
- Native `.nds` output for direct loading in melonDS or DeSmuME
- First boot check with a green top screen and cyan bottom screen

## Build

```bash
docker build -t helengine-ds .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-ds make
```

The build emits `build/helengine_ds.nds`.

## Generated core seam

The native build reserves `HELENGINE_CORE_CPP_ROOT` for later `cs2.cpp` integration, but the first milestone does not compile generated core output yet.

## Boot check

Load `build/helengine_ds.nds` in melonDS or DeSmuME. The expected result for this milestone is a green top screen and a cyan bottom screen with no immediate crash or reset loop.
````

- [ ] **Step 5: Commit the Docker/bootstrap documentation**

Run:

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds add Dockerfile README.md
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds commit -m "Add Nintendo DS Docker build scaffold"
```

Expected:

```text
A commit is created with the Docker image and first-pass README.
```

### Task 2: Native build script and entrypoint seam

**Files:**
- Create: `Makefile`
- Create: `src/main.cpp`

- [ ] **Step 1: Create the Nintendo DS build script with a reserved generated-core variable**

```makefile
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
```

- [ ] **Step 2: Create the tiny native entrypoint**

```cpp
#include "platform/ds/NintendoDsBootHost.hpp"

int main() {
    helengine::ds::NintendoDsBootHost host;
    return host.Run();
}
```

- [ ] **Step 3: Build the project before the host implementation exists**

Run:

```bash
rtk docker run --rm -v "$PWD":/workspace -w /workspace helengine-ds make
```

Expected:

```text
FAIL because src/platform/ds/NintendoDsBootHost.hpp and .cpp do not exist yet.
```

- [ ] **Step 4: Commit the build-script seam**

Run:

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds add Makefile src/main.cpp
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds commit -m "Add Nintendo DS native build entrypoint"
```

Expected:

```text
A commit is created with the Makefile and thin launcher.
```

### Task 3: Nintendo DS dual-screen bootstrap and verification host

**Files:**
- Create: `src/platform/ds/NintendoDsBootHost.hpp`
- Create: `src/platform/ds/NintendoDsBootHost.cpp`

- [ ] **Step 1: Create the host declaration with explicit display state**

```cpp
#pragma once

#include <nds.h>

namespace helengine::ds {
    /// Owns the first Nintendo DS native video bootstrap and verification frame loop.
    class NintendoDsBootHost {
    public:
        /// Creates the Nintendo DS bootstrap host with no initialized background state.
        NintendoDsBootHost();

        /// Initializes both DS displays and presents the verification colors until shutdown.
        int Run();

    private:
        static constexpr int FrameBufferWidth = 256;
        static constexpr int FrameBufferHeight = 256;
        static constexpr int FrameBufferPixelCount = FrameBufferWidth * FrameBufferHeight;
        static constexpr u16 TopScreenColor = RGB15(0, 31, 0);
        static constexpr u16 BottomScreenColor = RGB15(0, 31, 31);

        int MainBackgroundId;
        int SubBackgroundId;
        u16* MainFrameBuffer;
        u16* SubFrameBuffer;

        void InitializeVideo();
        void PaintVerificationColors();
    };
}
```

- [ ] **Step 2: Create the host implementation that paints both displays**

```cpp
#include "platform/ds/NintendoDsBootHost.hpp"

#include <algorithm>

namespace helengine::ds {
    NintendoDsBootHost::NintendoDsBootHost()
        : MainBackgroundId(-1),
          SubBackgroundId(-1),
          MainFrameBuffer(nullptr),
          SubFrameBuffer(nullptr) {
    }

    int NintendoDsBootHost::Run() {
        powerOn(POWER_ALL_2D);
        InitializeVideo();
        PaintVerificationColors();

        while (true) {
            swiWaitForVBlank();
        }

        return 0;
    }

    void NintendoDsBootHost::InitializeVideo() {
        videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
        videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

        vramSetBankA(VRAM_A_MAIN_BG);
        vramSetBankC(VRAM_C_SUB_BG);

        MainBackgroundId = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
        SubBackgroundId = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

        MainFrameBuffer = static_cast<u16*>(bgGetGfxPtr(MainBackgroundId));
        SubFrameBuffer = static_cast<u16*>(bgGetGfxPtr(SubBackgroundId));
    }

    void NintendoDsBootHost::PaintVerificationColors() {
        std::fill_n(MainFrameBuffer, FrameBufferPixelCount, TopScreenColor);
        std::fill_n(SubFrameBuffer, FrameBufferPixelCount, BottomScreenColor);
        swiWaitForVBlank();
    }
}
```

- [ ] **Step 3: Build the `.nds` artifact and verify the Dockerized build passes**

Run:

```bash
rtk docker run --rm -v "$PWD":/workspace -w /workspace helengine-ds make
```

Expected:

```text
The build succeeds and produces build/helengine_ds.nds.
```

- [ ] **Step 4: Verify the manual emulator boot target**

Run:

```bash
Open build/helengine_ds.nds in melonDS or DeSmuME.
```

Expected:

```text
The top screen is solid green, the bottom screen is solid cyan, and the process remains stable.
```

- [ ] **Step 5: Commit the first DS host bootstrap**

Run:

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds add src/platform/ds/NintendoDsBootHost.hpp src/platform/ds/NintendoDsBootHost.cpp
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds commit -m "Add Nintendo DS boot host scaffold"
```

Expected:

```text
A commit is created with the first dual-screen Nintendo DS bootstrap host.
```
