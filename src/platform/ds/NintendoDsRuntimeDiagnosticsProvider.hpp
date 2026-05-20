#pragma once

#include <nds/arm9/console.h>

#include <string>

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
class IRuntimeDiagnosticsProvider;
class IRuntimeEntityDisposalDiagnosticsProvider;
class IRuntimeSceneTransitionDiagnosticsProvider;
class IRuntimeUpdateStageDiagnosticsProvider;
class RuntimeMemoryDiagnosticsSnapshot;

#include "IRuntimeDiagnosticsProvider.hpp"
#include "IRuntimeEntityDisposalDiagnosticsProvider.hpp"
#include "IRuntimeSceneTransitionDiagnosticsProvider.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"

namespace helengine::ds {
    /// Supplies Nintendo DS runtime diagnostics to the generated core and mirrors update-stage changes onto the status console.
    class NintendoDsRuntimeDiagnosticsProvider final : public ::IRuntimeDiagnosticsProvider, public ::IRuntimeUpdateStageDiagnosticsProvider, public ::IRuntimeSceneTransitionDiagnosticsProvider, public ::IRuntimeEntityDisposalDiagnosticsProvider {
    public:
        /// Creates one diagnostics provider that writes live update-stage rows to the supplied DS console.
        /// <param name="statusConsole">Console initialized by the boot host for bottom-screen diagnostics.</param>
        explicit NintendoDsRuntimeDiagnosticsProvider(PrintConsole* statusConsole);

        /// Captures one Nintendo DS memory diagnostics snapshot for shared runtime diagnostics consumers.
        /// <returns>Allocated diagnostics snapshot populated with Nintendo DS allocator counters.</returns>
        ::RuntimeMemoryDiagnosticsSnapshot* CaptureSnapshot() override;

        /// Reports the next generated-core update stage directly to the bottom-screen status console.
        /// <param name="stage">Short update-stage label emitted by <c>Core</c>.</param>
        void ReportUpdateStage(std::string stage) override;

        /// Reports the next scene-manager transition stage directly to the bottom-screen status console.
        /// <param name="stage">Short scene-manager stage emitted by <c>SceneManager</c>.</param>
        /// <param name="sceneId">Stable scene id associated with the transition.</param>
        /// <param name="loadedSceneCount">Loaded scene count at the transition boundary.</param>
        /// <param name="pendingOperationCount">Pending operation count at the transition boundary.</param>
        void ReportSceneTransitionStage(std::string stage, std::string sceneId, int32_t loadedSceneCount, int32_t pendingOperationCount) override;

        /// Reports one entity disposal stage directly to the bottom-screen status console.
        /// <param name="stage">Short entity disposal stage label.</param>
        /// <param name="entityChildCount">Current child count for the entity being disposed.</param>
        /// <param name="componentCount">Current component count for the entity being disposed.</param>
        /// <param name="componentIndex">Component index involved in the stage, or -1 when not component-specific.</param>
        void ReportEntityDisposalStage(std::string stage, int32_t entityChildCount, int32_t componentCount, int32_t componentIndex) override;

    private:
        /// Stores the status console that receives live update-stage rows.
        PrintConsole* StatusConsole;

        /// Writes one padded diagnostics row to the configured status console.
        /// <param name="row">One-based console row to update.</param>
        /// <param name="text">Text that should replace the row contents.</param>
        void PrintStatusLine(int row, const char* text);
    };
}
#endif
