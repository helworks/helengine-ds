namespace helengine.ds.builder;

/// <summary>
/// Defines the native Nintendo DS build executor seam used by the managed builder.
/// </summary>
public interface INintendoDsNativeBuildExecutor {
    /// <summary>
    /// Builds one native Nintendo DS package for the provided workspace.
    /// </summary>
    /// <param name="workspace">Resolved build workspace to package.</param>
    /// <param name="cancellationToken">Cancellation token used to stop the build cooperatively.</param>
    void Build(NintendoDsBuildWorkspace workspace, CancellationToken cancellationToken);
}
