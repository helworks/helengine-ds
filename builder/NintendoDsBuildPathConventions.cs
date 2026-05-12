namespace helengine.ds.builder;

/// <summary>
/// Defines the builder-owned working-root path conventions used by Nintendo DS builds.
/// </summary>
internal static class NintendoDsBuildPathConventions {
    /// <summary>
    /// Stores the working-root child directory that contains the staged package source consumed by the builder.
    /// </summary>
    public const string PackageSourceDirectoryName = "package-source";

    /// <summary>
    /// Resolves the builder-owned staged package source root from one Nintendo DS working root.
    /// </summary>
    /// <param name="workingRootPath">Working root used for the Nintendo DS build.</param>
    /// <returns>Absolute staged package source root path.</returns>
    public static string ResolvePackageSourceRootPath(string workingRootPath) {
        if (string.IsNullOrWhiteSpace(workingRootPath)) {
            throw new ArgumentException("Working root path must be provided.", nameof(workingRootPath));
        }

        return Path.Combine(Path.GetFullPath(workingRootPath), PackageSourceDirectoryName);
    }
}
