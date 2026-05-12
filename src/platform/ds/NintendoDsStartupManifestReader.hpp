#pragma once

extern "C" {
#include <filesystem.h>
#include <nds/ndstypes.h>
}

namespace helengine::ds {
    /// <summary>
    /// Reads the packaged Nintendo DS startup manifest from NitroFS and validates its binary contract.
    /// </summary>
    class NintendoDsStartupManifestReader {
    public:
        /// <summary>
        /// Identifies the outcome of one startup-manifest read attempt.
        /// </summary>
        enum class Status {
            Success,
            MountFailed,
            FileMissing,
            InvalidMagic,
            UnsupportedVersion,
            InvalidPayloadSize,
            ReadFailed
        };

        /// <summary>
        /// Stores the result of one startup-manifest read attempt.
        /// </summary>
        struct Result {
            /// <summary>
            /// Gets the final manifest read status.
            /// </summary>
            Status ReadStatus;

            /// <summary>
            /// Gets the resolved top-screen color when the read succeeds.
            /// </summary>
            u16 TopScreenColor;

            /// <summary>
            /// Gets the resolved bottom-screen color when the read succeeds.
            /// </summary>
            u16 BottomScreenColor;
        };

        /// <summary>
        /// Reads the packaged startup manifest from NitroFS.
        /// </summary>
        /// <returns>Manifest read result containing the status and any resolved colors.</returns>
        Result Read() const;
    };
}
