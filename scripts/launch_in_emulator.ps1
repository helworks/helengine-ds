[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ArtifactPath,

    [Parameter()]
    [string]$MelonDsPath = "C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe",

    [Parameter()]
    [string]$LogDirectory = "C:\tmp\helengine-ds-logs"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$ResolvedArtifactPath = [System.IO.Path]::GetFullPath($ArtifactPath)
if (-not (Test-Path -LiteralPath $ResolvedArtifactPath -PathType Leaf)) {
    [Console]::Error.WriteLine("Artifact file was not found at '$ResolvedArtifactPath'.")
    exit 3
}

if ([System.IO.Path]::GetExtension($ResolvedArtifactPath) -ine '.nds') {
    [Console]::Error.WriteLine("Expected a .nds artifact at '$ResolvedArtifactPath'.")
    exit 4
}

if ([string]::IsNullOrWhiteSpace($MelonDsPath)) {
    [Console]::Error.WriteLine("MelonDsPath is required.")
    exit 5
}

$ResolvedMelonDsPath = [System.IO.Path]::GetFullPath($MelonDsPath)
if (-not (Test-Path -LiteralPath $ResolvedMelonDsPath -PathType Leaf)) {
    [Console]::Error.WriteLine("melonDS executable was not found at '$ResolvedMelonDsPath'.")
    exit 6
}

if ([string]::IsNullOrWhiteSpace($LogDirectory)) {
    [Console]::Error.WriteLine("LogDirectory is required.")
    exit 7
}

$ResolvedLogDirectory = [System.IO.Path]::GetFullPath($LogDirectory)
if (-not (Test-Path -LiteralPath $ResolvedLogDirectory -PathType Container)) {
    New-Item -ItemType Directory -Path $ResolvedLogDirectory -Force | Out-Null
}

$StdoutLogPath = Join-Path $ResolvedLogDirectory "melonDS-stdout.log"
$StderrLogPath = Join-Path $ResolvedLogDirectory "melonDS-stderr.log"
$BootTraceLogPath = Join-Path $ResolvedLogDirectory "helengine-ds-boot.log"
$SceneTransitionTraceLogPath = Join-Path $ResolvedLogDirectory "helengine-ds-scene-transition-trace.log"
Remove-Item -LiteralPath $StdoutLogPath, $StderrLogPath, $BootTraceLogPath, $SceneTransitionTraceLogPath -Force -ErrorAction SilentlyContinue

$ArtifactFile = Get-Item -LiteralPath $ResolvedArtifactPath
Write-Output ("ARTIFACT=" + $ArtifactFile.FullName)
Write-Output ("ARTIFACT_LAST_WRITE_TIME=" + $ArtifactFile.LastWriteTime.ToString("O"))
Write-Output ("EMULATOR=" + $ResolvedMelonDsPath)
Write-Output ("STDOUT_LOG=" + $StdoutLogPath)
Write-Output ("STDERR_LOG=" + $StderrLogPath)
Write-Output ("BOOT_LOG=" + $BootTraceLogPath)
Write-Output ("SCENE_TRACE_LOG=" + $SceneTransitionTraceLogPath)

try {
    $ExistingMelonDsProcesses = @(Get-Process -Name "melonDS" -ErrorAction SilentlyContinue)
    foreach ($ExistingMelonDsProcess in $ExistingMelonDsProcesses) {
        Stop-Process -Id $ExistingMelonDsProcess.Id -Force -ErrorAction Stop
    }

    foreach ($ExistingMelonDsProcess in $ExistingMelonDsProcesses) {
        try {
            Wait-Process -Id $ExistingMelonDsProcess.Id -Timeout 5 -ErrorAction Stop
        } catch {
            [Console]::Error.WriteLine("Failed to close existing melonDS process '" + $ExistingMelonDsProcess.Id + "': " + $_.Exception.Message)
            exit 8
        }
    }

    $MelonDsProcess = Start-Process -FilePath $ResolvedMelonDsPath -ArgumentList @($ResolvedArtifactPath) -WorkingDirectory (Split-Path $ResolvedMelonDsPath) -RedirectStandardOutput $StdoutLogPath -RedirectStandardError $StderrLogPath -PassThru
    Write-Output ("PROCESS_ID=" + $MelonDsProcess.Id)
} catch {
    [Console]::Error.WriteLine("Failed to launch melonDS: " + $_.Exception.Message)
    exit 9
}

exit 0
