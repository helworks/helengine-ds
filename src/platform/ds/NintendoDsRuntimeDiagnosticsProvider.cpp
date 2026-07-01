#include "platform/ds/NintendoDsRuntimeDiagnosticsProvider.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <cstdio>

#include "RuntimeMemoryDiagnosticsSnapshot.hpp"
#include "system/io/file-stream.hpp"
#include "system/io/file.hpp"
#include "FileMode.hpp"
#include "platform/ds/NintendoDsAllocationDiagnostics.hpp"

namespace helengine::ds {
    namespace {
        constexpr const char* SceneTransitionTracePath = "C:/tmp/helengine-ds-scene-transition-trace.log";
        bool SceneTransitionTraceReset = false;

        /// Appends one line to the host-side scene transition trace file without affecting gameplay behavior on failure.
        /// <param name="line">Trace payload to append.</param>
        void AppendSceneTransitionTraceLine(const std::string& line) {
            try {
                if (!SceneTransitionTraceReset) {
                    ::File::Delete(SceneTransitionTracePath);
                    SceneTransitionTraceReset = true;
                }

                ::FileStream stream(SceneTransitionTracePath, ::FileMode::Append);
                stream.Write(reinterpret_cast<const uint8_t*>(line.data()), 0, line.size());
                uint8_t newline = static_cast<uint8_t>('\n');
                stream.Write(&newline, 0, 1);
                stream.Flush();
                stream.Close();
            } catch (...) {
            }
        }
    }

    /// Creates one diagnostics provider that writes live update-stage rows to the supplied DS console.
    NintendoDsRuntimeDiagnosticsProvider::NintendoDsRuntimeDiagnosticsProvider(PrintConsole* statusConsole)
        : StatusConsole(statusConsole) {
    }

    /// Captures one Nintendo DS memory diagnostics snapshot for shared runtime diagnostics consumers.
    ::RuntimeMemoryDiagnosticsSnapshot* NintendoDsRuntimeDiagnosticsProvider::CaptureSnapshot() {
        ::RuntimeMemoryDiagnosticsSnapshot* snapshot = new ::RuntimeMemoryDiagnosticsSnapshot();
        snapshot->set_CommittedBytes(static_cast<uint64_t>(NintendoDsAllocationDiagnostics::GetCurrentAllocatedSize()));
        snapshot->set_PeakCommittedBytes(static_cast<uint64_t>(NintendoDsAllocationDiagnostics::GetPeakAllocatedSize()));
        snapshot->set_ResidentBytes(static_cast<uint64_t>(NintendoDsAllocationDiagnostics::GetCurrentAllocatedSize()));
        snapshot->set_PeakResidentBytes(static_cast<uint64_t>(NintendoDsAllocationDiagnostics::GetPeakAllocatedSize()));
        return snapshot;
    }

    /// Ignores generated-core update stages so normal diagnostics stay focused on version and allocation data.
    void NintendoDsRuntimeDiagnosticsProvider::ReportUpdateStage(std::string stage) {
        (void)stage;
    }

    /// Ignores scene transition stages so normal diagnostics stay focused on version and allocation data.
    void NintendoDsRuntimeDiagnosticsProvider::ReportSceneTransitionStage(std::string stage, std::string sceneId, int32_t loadedSceneCount, int32_t pendingOperationCount) {
        std::string line = "[helengine-ds] stage=" + stage
            + " scene=" + sceneId
            + " loaded=" + std::to_string(loadedSceneCount)
            + " pending=" + std::to_string(pendingOperationCount);
        AppendSceneTransitionTraceLine(line);
    }

    /// Ignores entity disposal stages so normal diagnostics stay focused on version and allocation data.
    void NintendoDsRuntimeDiagnosticsProvider::ReportEntityDisposalStage(std::string stage, int32_t entityChildCount, int32_t componentCount, int32_t componentIndex) {
        (void)stage;
        (void)entityChildCount;
        (void)componentCount;
        (void)componentIndex;
    }

    /// Writes one padded diagnostics row to the configured status console.
    void NintendoDsRuntimeDiagnosticsProvider::PrintStatusLine(int row, const char* text) {
        if (StatusConsole == nullptr) {
            return;
        }

        consoleSelect(StatusConsole);
        iprintf("\x1b[%d;0H%-32.32s", row, text != nullptr ? text : "");
    }
}
#endif
