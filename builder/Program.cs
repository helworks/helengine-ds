using System.Runtime.Versioning;

namespace helengine.ds.builder;

/// <summary>
/// Provides a small command-line entrypoint for the Nintendo DS builder assembly.
/// </summary>
[SupportedOSPlatform("windows")]
public static class Program {
    /// <summary>
    /// Prints builder metadata or runs the builder smoke mode.
    /// </summary>
    /// <param name="args">Command-line arguments.</param>
    /// <returns>Zero on success.</returns>
    public static int Main(string[] args) {
        if (args.Length > 0 && string.Equals(args[0], "--describe", StringComparison.OrdinalIgnoreCase)) {
            NintendoDsPlatformAssetBuilder builder = new();
            Console.WriteLine(builder.Descriptor.BuilderId);
            Console.WriteLine(builder.Descriptor.TargetPlatformId);
            Console.WriteLine(builder.Definition.DisplayName);
            return 0;
        }

        if (args.Length > 0 && string.Equals(args[0], "--smoke-test", StringComparison.OrdinalIgnoreCase)) {
            NintendoDsVerificationHarness.RunSmokeTest();
            return 0;
        }

        if (args.Length > 0 && string.Equals(args[0], "--verify-native", StringComparison.OrdinalIgnoreCase)) {
            return VerifyNative(args);
        }

        Console.WriteLine("helengine.ds.builder --describe | --smoke-test | --verify-native <generated-core-root> <staging-root> <output-root>");
        return 0;
    }

    /// <summary>
    /// Runs the smoke-first DS native verification flow from command-line arguments.
    /// </summary>
    /// <param name="args">Command-line arguments.</param>
    /// <returns>Zero on success; one when arguments are invalid.</returns>
    static int VerifyNative(string[] args) {
        if (args.Length < 4 || string.IsNullOrWhiteSpace(args[1]) || string.IsNullOrWhiteSpace(args[2]) || string.IsNullOrWhiteSpace(args[3])) {
            Console.Error.WriteLine("Usage: helengine.ds.builder --verify-native <generated-core-root> <staging-root> <output-root>");
            return 1;
        }

        try {
            NintendoDsVerificationHarness.RunNativeVerification(args[1], args[2], args[3]);
            return 0;
        } catch (Exception exception) {
            Console.Error.WriteLine(exception.Message);
            return 1;
        }
    }
}
