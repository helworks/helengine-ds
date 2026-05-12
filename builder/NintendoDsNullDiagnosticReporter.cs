using helengine.baseplatform.Builders;
using helengine.baseplatform.Reporting;

namespace helengine.ds.builder;

/// <summary>
/// Discards streamed builder diagnostics for command-line smoke runs.
/// </summary>
internal sealed class NintendoDsNullDiagnosticReporter : IPlatformBuildDiagnosticReporter {
    /// <summary>
    /// Ignores one streamed diagnostic emitted during the smoke build.
    /// </summary>
    /// <param name="diagnostic">Diagnostic emitted by the builder.</param>
    public void Report(PlatformBuildDiagnostic diagnostic) {
    }
}
