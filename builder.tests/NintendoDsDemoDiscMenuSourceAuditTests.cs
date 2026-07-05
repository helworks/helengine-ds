namespace helengine.ds.builder.tests {
    /// <summary>
    /// Verifies the city-owned demo-disc menu factory can restore the Nintendo DS logo animation while leaving the renderer free to choose the active DS OBJ tiling strategy.
    /// </summary>
    public sealed class NintendoDsDemoDiscMenuSourceAuditTests {
        /// <summary>
        /// Verifies the Nintendo DS logo entity attaches the shared animation player clip instead of freezing a static seam-probe pose.
        /// </summary>
        [Fact]
        public void Source_whenAuthoringNintendoDsLogo_restoresSharedAnimationClip() {
            string sourcePath = @"C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs";
            string sourceCode = File.ReadAllText(sourcePath);
            int logoMethodStart = sourceCode.IndexOf("void CreateNintendoDsTopScreenLogoEntity(Entity topScreenRootEntity, MenuOverlayImageDefinition overlayImage) {", StringComparison.Ordinal);
            int nextMethodStart = sourceCode.IndexOf("void CreateNintendoDsTopScreenPlatformInfoEntity(Entity topScreenRootEntity, MenuDefinition definition) {", StringComparison.Ordinal);
            string logoMethodBody = sourceCode[logoMethodStart..nextMethodStart];

            Assert.Contains("const string DemoDiscLogoIdleAnimationRelativePath = \"Animations/DemoDiscLogoIdle.hanim\";", sourceCode, StringComparison.Ordinal);
            Assert.Contains("entity.LocalScale = new float3(1f, 1f, 1f);", logoMethodBody, StringComparison.Ordinal);
            Assert.Contains("entity.LocalOrientation = float4.Identity;", logoMethodBody, StringComparison.Ordinal);
            Assert.Contains("AnimationPlayerComponent animationPlayerComponent = new AnimationPlayerComponent", logoMethodBody, StringComparison.Ordinal);
            Assert.Contains("Clip = LoadRequiredAnimationClipAsset(DemoDiscLogoIdleAnimationRelativePath),", logoMethodBody, StringComparison.Ordinal);
            Assert.Contains("PlayAutomatically = true,", logoMethodBody, StringComparison.Ordinal);
            Assert.Contains("ShouldLoop = true", logoMethodBody, StringComparison.Ordinal);
            Assert.Contains("ApplyAnimationClipReference(entity, animationPlayerComponent, DemoDiscLogoIdleAnimationRelativePath);", logoMethodBody, StringComparison.Ordinal);
            Assert.DoesNotContain("NintendoDsLogoDebugAngleRadians", sourceCode, StringComparison.Ordinal);
        }
    }
}
