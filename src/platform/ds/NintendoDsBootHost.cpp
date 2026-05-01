#include "platform/ds/NintendoDsBootHost.hpp"

#include <algorithm>

namespace helengine::ds {
    /// Creates the Nintendo DS bootstrap host with no initialized background state.
    NintendoDsBootHost::NintendoDsBootHost()
        : MainBackgroundId(-1)
        , SubBackgroundId(-1)
        , MainFrameBuffer(nullptr)
        , SubFrameBuffer(nullptr) {
    }

    /// Initializes both DS displays and presents the verification colors until shutdown.
    int NintendoDsBootHost::Run() {
        powerOn(POWER_ALL_2D);

        if (!InitializeVideo()) {
            return 1;
        }

        PaintVerificationColors();

        while (true) {
            swiWaitForVBlank();
        }

        return 0;
    }

    /// Initializes the DS video mode, VRAM routing, and bitmap backgrounds.
    bool NintendoDsBootHost::InitializeVideo() {
        videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
        videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

        vramSetBankA(VRAM_A_MAIN_BG);
        vramSetBankC(VRAM_C_SUB_BG);

        MainBackgroundId = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
        SubBackgroundId = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

        if (MainBackgroundId < 0) {
            return false;
        } else if (SubBackgroundId < 0) {
            return false;
        }

        MainFrameBuffer = static_cast<u16*>(bgGetGfxPtr(MainBackgroundId));
        SubFrameBuffer = static_cast<u16*>(bgGetGfxPtr(SubBackgroundId));

        if (MainFrameBuffer == nullptr) {
            return false;
        } else if (SubFrameBuffer == nullptr) {
            return false;
        }

        return true;
    }

    /// Paints the verification colors to the top and bottom display framebuffers.
    void NintendoDsBootHost::PaintVerificationColors() {
        std::fill_n(MainFrameBuffer, FrameBufferPixelCount, TopScreenColor);
        std::fill_n(SubFrameBuffer, FrameBufferPixelCount, BottomScreenColor);
        swiWaitForVBlank();
    }
}
