namespace helengine.ds.scratch.tilepreview;

/// <summary>
/// Resolves a reasonable default DS build workspace path for the tile preview tool.
/// </summary>
public static class LatestDsWorkspaceLocator {
    /// <summary>
    /// Finds the newest helengine DS temp build workspace when one exists.
    /// </summary>
    /// <returns>Newest workspace path, or an empty string when no temp workspace exists.</returns>
    public static string ResolveInitialPath() {
        string tempRootPath = Path.Combine(Path.GetTempPath(), "helengine-platform-build", "ds");
        if (!Directory.Exists(tempRootPath)) {
            return string.Empty;
        }

        DirectoryInfo newestWorkspaceDirectory = null;
        foreach (DirectoryInfo candidateDirectory in new DirectoryInfo(tempRootPath).GetDirectories()) {
            if (newestWorkspaceDirectory == null || candidateDirectory.LastWriteTimeUtc > newestWorkspaceDirectory.LastWriteTimeUtc) {
                newestWorkspaceDirectory = candidateDirectory;
            }
        }

        return newestWorkspaceDirectory == null ? string.Empty : newestWorkspaceDirectory.FullName;
    }
}
