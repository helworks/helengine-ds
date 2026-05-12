#include "platform/ds/NintendoDsStartupManifestReader.hpp"

#include <cstdio>

namespace helengine::ds {
    namespace {
        constexpr char StartupManifestPath[] = "nitro:/runtime/ds_startup_manifest.bin";
        constexpr unsigned StartupManifestVersion = 1;
        constexpr unsigned StartupManifestPayloadSize = 4;
    }

    /// <summary>
    /// Reads the packaged startup manifest from NitroFS.
    /// </summary>
    /// <returns>Manifest read result containing the status and any resolved colors.</returns>
    NintendoDsStartupManifestReader::Result NintendoDsStartupManifestReader::Read() const {
        if (!nitroFSInit(nullptr)) {
            return Result { Status::MountFailed, 0, 0 };
        }

        FILE* file = fopen(StartupManifestPath, "rb");
        if (file == nullptr) {
            return Result { Status::FileMissing, 0, 0 };
        }

        unsigned char magic[4];
        if (fread(magic, 1, sizeof(magic), file) != sizeof(magic)) {
            fclose(file);
            return Result { Status::ReadFailed, 0, 0 };
        }

        if (magic[0] != 0x48 || magic[1] != 0x44 || magic[2] != 0x53 || magic[3] != 0x50) {
            fclose(file);
            return Result { Status::InvalidMagic, 0, 0 };
        }

        unsigned short version = 0;
        unsigned short payloadSize = 0;
        unsigned short topScreenColor = 0;
        unsigned short bottomScreenColor = 0;
        if (fread(&version, sizeof(version), 1, file) != 1
            || fread(&payloadSize, sizeof(payloadSize), 1, file) != 1
            || fread(&topScreenColor, sizeof(topScreenColor), 1, file) != 1
            || fread(&bottomScreenColor, sizeof(bottomScreenColor), 1, file) != 1) {
            fclose(file);
            return Result { Status::ReadFailed, 0, 0 };
        }

        fclose(file);

        if (version != StartupManifestVersion) {
            return Result { Status::UnsupportedVersion, 0, 0 };
        } else if (payloadSize != StartupManifestPayloadSize) {
            return Result { Status::InvalidPayloadSize, 0, 0 };
        }

        return Result { Status::Success, topScreenColor, bottomScreenColor };
    }
}
