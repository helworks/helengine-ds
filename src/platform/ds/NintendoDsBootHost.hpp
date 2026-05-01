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
        /// Stores the width of the 16-bit bitmap background used for each display.
        static constexpr int FrameBufferWidth = 256;

        /// Stores the height of the 16-bit bitmap background used for each display.
        static constexpr int FrameBufferHeight = 256;

        /// Stores the number of pixels written for each full-screen verification frame.
        static constexpr int FrameBufferPixelCount = FrameBufferWidth * FrameBufferHeight;

        /// Stores the top-screen verification color with the bitmap visibility bit enabled.
        static constexpr u16 TopScreenColor = RGB15(0, 31, 0) | BIT(15);

        /// Stores the bottom-screen verification color with the bitmap visibility bit enabled.
        static constexpr u16 BottomScreenColor = RGB15(0, 31, 31) | BIT(15);

        /// Stores the main-engine background slot used for the top screen.
        int MainBackgroundId;

        /// Stores the sub-engine background slot used for the bottom screen.
        int SubBackgroundId;

        /// Stores the top-screen framebuffer pointer.
        u16* MainFrameBuffer;

        /// Stores the bottom-screen framebuffer pointer.
        u16* SubFrameBuffer;

        /// Initializes the DS video mode, VRAM routing, and bitmap backgrounds.
        bool InitializeVideo();

        /// Paints the verification colors to the top and bottom display framebuffers.
        void PaintVerificationColors();
    };
}
