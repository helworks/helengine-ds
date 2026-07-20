#include "platform/ds/audio/NintendoDsAudioBackend.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>

extern "C" {
#include <nds/arm9/sound.h>
}

namespace helengine::ds {
    namespace {
        /// Uses the libnds center pan value for all Helengine DS sample playback.
        constexpr u8 CenterPan = 64;

        /// Stores the maximum libnds channel volume accepted by the sample playback helpers.
        constexpr int32_t MaxHardwareVolume = 127;

        /// Stores the four-byte Nintendo DS IMA ADPCM header size that must be skipped when looping from the beginning.
        constexpr int32_t ImaAdpcmHeaderByteLength = 4;

        /// Determines whether one cooked audio asset carries the DS buffered ADPCM payload family.
        /// <param name="asset">Audio asset to inspect.</param>
        /// <returns>True when the asset should play through the native IMA ADPCM format.</returns>
        bool UsesImaAdpcmEncoding(const ::AudioAsset& asset) {
            return asset.EncodingFamilyId == "adpcm-buffered";
        }
    }

    /// Creates one Nintendo DS audio backend and enables sound playback.
    NintendoDsAudioBackend::NintendoDsAudioBackend()
        : NextVoiceId(0)
        , ActiveVoicesById()
        , BusGainsById()
        , PausedBusIds() {
        soundEnable();
        BusGainsById.emplace("master", 1.0f);
        BusGainsById.emplace("music", 1.0f);
        BusGainsById.emplace("sfx", 1.0f);
    }

    /// Stops all active voices and releases any native audio buffers owned by the backend.
    NintendoDsAudioBackend::~NintendoDsAudioBackend() {
        for (auto& voiceEntry : ActiveVoicesById) {
            ReleaseVoiceState(voiceEntry.second, true);
        }

        ActiveVoicesById.clear();
    }

    int32_t NintendoDsAudioBackend::Play(::AudioAsset* asset, ::AudioPlaybackRequest* request) {
        if (asset == nullptr) {
            throw std::invalid_argument("asset");
        }
        if (asset->SampleRate <= 0 || asset->SampleRate > 65535) {
            throw std::runtime_error("Nintendo DS audio playback requires a positive sample rate that fits the native API.");
        }
        if (asset->Channels != 1) {
            throw std::runtime_error("Nintendo DS audio playback currently requires mono cooked assets.");
        }
        if (asset->EncodedBytes == nullptr || asset->EncodedBytes->Length <= 0 || asset->EncodedBytes->Data == nullptr) {
            throw std::runtime_error("Nintendo DS audio playback requires one non-empty encoded payload.");
        }
        const bool useImaAdpcm = UsesImaAdpcmEncoding(*asset);
        if (!useImaAdpcm && (asset->EncodedBytes->Length % static_cast<int32_t>(sizeof(std::int16_t))) != 0) {
            throw std::runtime_error("Nintendo DS audio playback requires 16-bit PCM sample alignment.");
        }
        if (useImaAdpcm && asset->EncodedBytes->Length < ImaAdpcmHeaderByteLength) {
            throw std::runtime_error("Nintendo DS audio playback requires an IMA ADPCM payload header.");
        }

        const int32_t paddedByteLength = PadByteLengthForHardware(asset->EncodedBytes->Length);
        void* audioBuffer = std::malloc(static_cast<std::size_t>(paddedByteLength));
        if (audioBuffer == nullptr) {
            throw std::runtime_error("Nintendo DS audio playback could not allocate one native audio buffer.");
        }

        std::memset(audioBuffer, 0, static_cast<std::size_t>(paddedByteLength));
        std::memcpy(audioBuffer, asset->EncodedBytes->Data, static_cast<std::size_t>(asset->EncodedBytes->Length));

        const std::string busId = NormalizeBusId(
            request != nullptr && !request->BusId.empty()
                ? request->BusId
                : asset->DefaultBusId);
        const float baseGain = ClampGain(request != nullptr ? request->Gain : 1.0f);
        const bool loop = request != nullptr ? request->Loop : asset->DefaultLoop;
        const int32_t volume = ConvertGainToVolume(ResolveCombinedGain(busId, baseGain));
        const SoundFormat sampleFormat = useImaAdpcm ? SoundFormat_ADPCM : SoundFormat_16Bit;
        const u16 loopPoint = static_cast<u16>(loop && useImaAdpcm ? ImaAdpcmHeaderByteLength : 0);

        const int channelId = soundPlaySample(
            audioBuffer,
            sampleFormat,
            static_cast<u32>(paddedByteLength),
            static_cast<u16>(asset->SampleRate),
            static_cast<u8>(volume),
            CenterPan,
            loop,
            loopPoint);
        if (channelId < 0) {
            std::free(audioBuffer);
            throw std::runtime_error("Nintendo DS audio playback could not reserve one hardware sample channel.");
        }

        ActiveVoiceState voice = {};
        voice.VoiceId = NextVoiceId++;
        voice.ChannelId = channelId;
        voice.BusId = busId;
        voice.BaseGain = baseGain;
        voice.Looping = loop;
        voice.Paused = false;
        voice.AudioBuffer = audioBuffer;
        voice.AudioBufferByteLength = paddedByteLength;
        ActiveVoicesById.emplace(voice.VoiceId, voice);
        ApplyVoicePlaybackState(ActiveVoicesById.at(voice.VoiceId));
        return voice.VoiceId;
    }

    void NintendoDsAudioBackend::Stop(int32_t voiceId) {
        auto voiceIterator = ActiveVoicesById.find(voiceId);
        if (voiceIterator == ActiveVoicesById.end()) {
            return;
        }

        ReleaseVoiceState(voiceIterator->second, true);
        ActiveVoicesById.erase(voiceIterator);
    }

    void NintendoDsAudioBackend::SetBusGain(std::string busId, float gain) {
        BusGainsById[NormalizeBusId(std::move(busId))] = ClampGain(gain);
        for (auto& voiceEntry : ActiveVoicesById) {
            ApplyVoicePlaybackState(voiceEntry.second);
        }
    }

    void NintendoDsAudioBackend::SetBusPaused(std::string busId, bool paused) {
        std::string normalizedBusId = NormalizeBusId(std::move(busId));
        if (paused) {
            PausedBusIds.insert(normalizedBusId);
        } else {
            PausedBusIds.erase(normalizedBusId);
        }

        for (auto& voiceEntry : ActiveVoicesById) {
            ApplyVoicePlaybackState(voiceEntry.second);
        }
    }

    bool NintendoDsAudioBackend::IsPlaying(int32_t voiceId) {
        auto voiceIterator = ActiveVoicesById.find(voiceId);
        if (voiceIterator == ActiveVoicesById.end()) {
            return false;
        }

        if (voiceIterator->second.Paused) {
            return true;
        }

        const unsigned activeMask = soundGetActiveChannels();
        return (activeMask & (1U << voiceIterator->second.ChannelId)) != 0;
    }

    void NintendoDsAudioBackend::Update() {
        const unsigned activeMask = soundGetActiveChannels();
        for (auto voiceIterator = ActiveVoicesById.begin(); voiceIterator != ActiveVoicesById.end();) {
            ActiveVoiceState& voice = voiceIterator->second;
            if (voice.Paused) {
                ++voiceIterator;
                continue;
            }

            if ((activeMask & (1U << voice.ChannelId)) != 0) {
                ++voiceIterator;
                continue;
            }

            ReleaseVoiceState(voice, false);
            voiceIterator = ActiveVoicesById.erase(voiceIterator);
        }
    }

    /// Normalizes one mixer bus identifier into the canonical lowercase lookup form.
    /// <param name="busId">Mixer bus identifier to normalize.</param>
    /// <returns>Normalized mixer bus identifier.</returns>
    std::string NintendoDsAudioBackend::NormalizeBusId(std::string busId) {
        if (busId.empty()) {
            return "master";
        }

        std::transform(
            busId.begin(),
            busId.end(),
            busId.begin(),
            [](unsigned char value) {
                return static_cast<char>(std::tolower(value));
            });
        return busId;
    }

    /// Clamps one gain value into the DS runtime-supported range.
    /// <param name="gain">Mixer gain to clamp.</param>
    /// <returns>Clamped gain.</returns>
    float NintendoDsAudioBackend::ClampGain(float gain) {
        if (!(gain >= 0.0f) || gain != gain) {
            return 0.0f;
        }

        return std::clamp(gain, 0.0f, 1.0f);
    }

    /// Converts one normalized gain value into the DS hardware volume range.
    /// <param name="gain">Normalized gain value.</param>
    /// <returns>DS hardware volume value.</returns>
    int32_t NintendoDsAudioBackend::ConvertGainToVolume(float gain) {
        return static_cast<int32_t>(ClampGain(gain) * static_cast<float>(MaxHardwareVolume));
    }

    /// Rounds one byte count up to the DS sample engine's 32-bit size granularity.
    /// <param name="byteLength">Unpadded encoded byte length.</param>
    /// <returns>Padded byte length safe for DS sample playback.</returns>
    int32_t NintendoDsAudioBackend::PadByteLengthForHardware(int32_t byteLength) {
        if (byteLength <= 0) {
            return 0;
        }

        constexpr int32_t WordSize = 4;
        return ((byteLength + (WordSize - 1)) / WordSize) * WordSize;
    }

    /// Resolves the final gain after applying the master and per-bus mixer settings.
    /// <param name="busId">Normalized mixer bus identifier.</param>
    /// <param name="baseGain">Per-request gain before bus modifiers.</param>
    /// <returns>Combined gain applied to the hardware voice.</returns>
    float NintendoDsAudioBackend::ResolveCombinedGain(const std::string& busId, float baseGain) const {
        float masterGain = 1.0f;
        auto masterGainIterator = BusGainsById.find("master");
        if (masterGainIterator != BusGainsById.end()) {
            masterGain = masterGainIterator->second;
        }

        float busGain = 1.0f;
        auto busGainIterator = BusGainsById.find(busId);
        if (busGainIterator != BusGainsById.end()) {
            busGain = busGainIterator->second;
        }

        return ClampGain(masterGain * busGain * baseGain);
    }

    /// Determines whether one normalized mixer bus is currently paused.
    /// <param name="busId">Normalized mixer bus identifier.</param>
    /// <returns>True when the bus or master bus is paused.</returns>
    bool NintendoDsAudioBackend::IsBusPaused(const std::string& busId) const {
        return PausedBusIds.contains("master") || PausedBusIds.contains(busId);
    }

    /// Applies the current pause and volume state to one active DS hardware voice.
    /// <param name="voice">Voice state to update.</param>
    void NintendoDsAudioBackend::ApplyVoicePlaybackState(ActiveVoiceState& voice) {
        const bool shouldPause = IsBusPaused(voice.BusId);
        if (shouldPause != voice.Paused) {
            if (shouldPause) {
                soundPause(voice.ChannelId);
            } else {
                soundResume(voice.ChannelId);
            }

            voice.Paused = shouldPause;
        }

        soundSetVolume(voice.ChannelId, static_cast<u8>(ConvertGainToVolume(ResolveCombinedGain(voice.BusId, voice.BaseGain))));
    }

    /// Stops one active hardware voice and releases its owned PCM buffer.
    /// <param name="voice">Voice state to release.</param>
    /// <param name="stopNativeVoice">True when the DS channel should be stopped immediately.</param>
    void NintendoDsAudioBackend::ReleaseVoiceState(ActiveVoiceState& voice, bool stopNativeVoice) {
        if (stopNativeVoice) {
            soundKill(voice.ChannelId);
        }

        if (voice.AudioBuffer != nullptr) {
            std::free(voice.AudioBuffer);
            voice.AudioBuffer = nullptr;
            voice.AudioBufferByteLength = 0;
        }
    }
}
