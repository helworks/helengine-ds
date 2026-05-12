using helengine.baseplatform.Builders;
using helengine.baseplatform.Reporting;

namespace helengine.ds.builder;

/// <summary>
/// Discards streamed builder progress updates for command-line smoke runs.
/// </summary>
internal sealed class NintendoDsNullProgressReporter : IPlatformBuildProgressReporter {
    /// <summary>
    /// Ignores one streamed progress update emitted during the smoke build.
    /// </summary>
    /// <param name="update">Progress update emitted by the builder.</param>
    public void Report(PlatformBuildProgressUpdate update) {
    }
}
