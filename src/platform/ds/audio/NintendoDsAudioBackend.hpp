#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "AudioAsset.hpp"
#include "AudioPlaybackRequest.hpp"
#include "IAudioBackend.hpp"

namespace helengine::ds {
    /// <summary>
    /// Plays shared Helengine buffered audio payloads through the Nintendo DS sample playback channels.
    /// </summary>
    class NintendoDsAudioBackend final : public ::IAudioBackend {
    public:
        /// <summary>
        /// Creates one Nintendo DS audio backend and enables sound playback.
        /// </summary>
        NintendoDsAudioBackend();

        /// <summary>
        /// Stops all active voices and releases any native audio buffers owned by the backend.
        /// </summary>
        ~NintendoDsAudioBackend();

        int32_t Play(::AudioAsset* asset, ::AudioPlaybackRequest* request) override;

        void Stop(int32_t voiceId) override;

        void SetBusGain(std::string busId, float gain) override;

        void SetBusPaused(std::string busId, bool paused) override;

        bool IsPlaying(int32_t voiceId) override;

        void Update() override;

    private:
        /// <summary>
        /// Stores one DS hardware voice routed from the audio manager.
        /// </summary>
        struct ActiveVoiceState {
            int32_t VoiceId;
            int ChannelId;
            std::string BusId;
            float BaseGain;
            bool Looping;
            bool Paused;
            void* AudioBuffer;
            int32_t AudioBufferByteLength;
        };

        /// <summary>
        /// Normalizes one mixer bus identifier into the canonical lowercase lookup form.
        /// </summary>
        /// <param name="busId">Mixer bus identifier to normalize.</param>
        /// <returns>Normalized mixer bus identifier.</returns>
        static std::string NormalizeBusId(std::string busId);

        /// <summary>
        /// Clamps one gain value into the DS runtime-supported range.
        /// </summary>
        /// <param name="gain">Mixer gain to clamp.</param>
        /// <returns>Clamped gain.</returns>
        static float ClampGain(float gain);

        /// <summary>
        /// Converts one normalized gain value into the DS hardware volume range.
        /// </summary>
        /// <param name="gain">Normalized gain value.</param>
        /// <returns>DS hardware volume value.</returns>
        static int32_t ConvertGainToVolume(float gain);

        /// <summary>
        /// Rounds one byte count up to the DS sample engine's 32-bit size granularity.
        /// </summary>
        /// <param name="byteLength">Unpadded encoded byte length.</param>
        /// <returns>Padded byte length safe for DS sample playback.</returns>
        static int32_t PadByteLengthForHardware(int32_t byteLength);

        /// <summary>
        /// Resolves the final gain after applying the master and per-bus mixer settings.
        /// </summary>
        /// <param name="busId">Normalized mixer bus identifier.</param>
        /// <param name="baseGain">Per-request gain before bus modifiers.</param>
        /// <returns>Combined gain applied to the hardware voice.</returns>
        float ResolveCombinedGain(const std::string& busId, float baseGain) const;

        /// <summary>
        /// Determines whether one normalized mixer bus is currently paused.
        /// </summary>
        /// <param name="busId">Normalized mixer bus identifier.</param>
        /// <returns>True when the bus or master bus is paused.</returns>
        bool IsBusPaused(const std::string& busId) const;

        /// <summary>
        /// Applies the current pause and volume state to one active DS hardware voice.
        /// </summary>
        /// <param name="voice">Voice state to update.</param>
        void ApplyVoicePlaybackState(ActiveVoiceState& voice);

        /// <summary>
        /// Stops one active hardware voice and releases its owned PCM buffer.
        /// </summary>
        /// <param name="voice">Voice state to release.</param>
        /// <param name="stopNativeVoice">True when the DS channel should be stopped immediately.</param>
        void ReleaseVoiceState(ActiveVoiceState& voice, bool stopNativeVoice);

        /// <summary>
        /// Stores the next stable voice id returned to the engine audio manager.
        /// </summary>
        int32_t NextVoiceId;

        /// <summary>
        /// Stores currently active hardware voices keyed by the engine-visible voice id.
        /// </summary>
        std::unordered_map<int32_t, ActiveVoiceState> ActiveVoicesById;

        /// <summary>
        /// Stores the current gain multiplier for each normalized mixer bus id.
        /// </summary>
        std::unordered_map<std::string, float> BusGainsById;

        /// <summary>
        /// Stores the normalized mixer buses that should remain paused.
        /// </summary>
        std::unordered_set<std::string> PausedBusIds;
    };
}
