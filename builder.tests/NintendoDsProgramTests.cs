using System.Runtime.Versioning;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS builder command-line entrypoints.
/// </summary>
[Collection(NintendoDsConsoleSensitiveTestCollection.CollectionName)]
[SupportedOSPlatform("windows")]
public class NintendoDsProgramTests {
    /// <summary>
    /// Verifies the smoke-test entrypoint executes the builder-owned staging flow instead of printing the placeholder message.
    /// </summary>
    [Fact]
    public void Main_whenSmokeTestIsRequested_runs_builder_smoke_harness() {
        TextWriter previousOutput = Console.Out;
        StringWriter output = new StringWriter();

        try {
            Console.SetOut(output);

            int exitCode = Program.Main(["--smoke-test"]);

            Assert.Equal(0, exitCode);
            Assert.Contains("Smoke test passed.", output.ToString(), StringComparison.Ordinal);
        } finally {
            Console.SetOut(previousOutput);
            output.Dispose();
        }
    }

    /// <summary>
    /// Verifies the verify-native entrypoint returns usage information when required arguments are missing.
    /// </summary>
    [Fact]
    public void Main_whenVerifyNativeArgumentsAreMissing_returns_usage_and_exit_code_1() {
        TextWriter previousError = Console.Error;
        StringWriter error = new StringWriter();

        try {
            Console.SetError(error);

            int exitCode = Program.Main(["--verify-native"]);

            Assert.Equal(1, exitCode);
            Assert.Contains(
                "Usage: helengine.ds.builder --verify-native <generated-core-root> <staging-root> <output-root>",
                error.ToString(),
                StringComparison.Ordinal);
        } finally {
            Console.SetError(previousError);
            error.Dispose();
        }
    }

    /// <summary>
    /// Verifies the verify-native entrypoint reports builder verification failures as a non-zero exit code instead of crashing the process.
    /// </summary>
    [Fact]
    public void Main_whenVerifyNativeBuildFails_returns_exit_code_1_and_writes_error() {
        TextWriter previousError = Console.Error;
        StringWriter error = new StringWriter();
        string workingRootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-program-" + Guid.NewGuid().ToString("N"));
        string stagingRootPath = Path.Combine(workingRootPath, "staging");
        string outputRootPath = Path.Combine(workingRootPath, "out");

        try {
            Directory.CreateDirectory(stagingRootPath);
            Console.SetError(error);

            int exitCode = Program.Main([
                "--verify-native",
                Path.Combine(workingRootPath, "missing-generated-core"),
                stagingRootPath,
                outputRootPath
            ]);

            Assert.Equal(1, exitCode);
            Assert.Contains("Generated core root", error.ToString(), StringComparison.OrdinalIgnoreCase);
        } finally {
            Console.SetError(previousError);

            if (Directory.Exists(workingRootPath)) {
                Directory.Delete(workingRootPath, recursive: true);
            }

            error.Dispose();
        }
    }
}
