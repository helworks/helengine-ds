namespace helengine.ds.builder.tests;

/// <summary>
/// Guards the native Nintendo DS audio wiring contract across the boot host and Makefile.
/// </summary>
public sealed class NintendoDsAudioSourceContractTests {
    /// <summary>
    /// Ensures the Nintendo DS boot host constructs the native audio backend, assigns it to generated core, updates it during the main loop, and compiles the audio source directory.
    /// </summary>
    [Fact]
    public void Source_contract_wires_ds_audio_backend_into_boot_host_and_makefile() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string bootHostHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.hpp");
        string bootHostSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string backendHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "audio", "NintendoDsAudioBackend.hpp");
        string backendSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "audio", "NintendoDsAudioBackend.cpp");
        string makefilePath = Path.Combine(repositoryRootPath, "Makefile");

        Assert.True(File.Exists(bootHostHeaderPath), "Expected NintendoDsBootHost.hpp to exist.");
        Assert.True(File.Exists(bootHostSourcePath), "Expected NintendoDsBootHost.cpp to exist.");
        Assert.True(File.Exists(backendHeaderPath), "Expected NintendoDsAudioBackend.hpp to exist.");
        Assert.True(File.Exists(backendSourcePath), "Expected NintendoDsAudioBackend.cpp to exist.");
        Assert.True(File.Exists(makefilePath), "Expected Makefile to exist.");

        string bootHostHeaderSource = File.ReadAllText(bootHostHeaderPath);
        string bootHostSource = File.ReadAllText(bootHostSourcePath);
        string backendSource = File.ReadAllText(backendSourcePath);
        string makefileSource = File.ReadAllText(makefilePath);

        Assert.Contains("class NintendoDsAudioBackend;", bootHostHeaderSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsAudioBackend* EngineAudioBackend;", bootHostHeaderSource, StringComparison.Ordinal);
        Assert.Contains("platform/ds/audio/NintendoDsAudioBackend.hpp", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("EngineAudioBackend = new NintendoDsAudioBackend();", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("EngineCore->SetAudioBackend(EngineAudioBackend);", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("EngineAudioBackend->Update();", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("EncodingFamilyId == \"adpcm-buffered\"", backendSource, StringComparison.Ordinal);
        Assert.Contains("SoundFormat_ADPCM", backendSource, StringComparison.Ordinal);
        Assert.Contains("src/platform/ds/audio", makefileSource, StringComparison.Ordinal);
    }
}
