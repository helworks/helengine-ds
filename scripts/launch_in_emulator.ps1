[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ArtifactPath,

    [Parameter()]
    [string]$MelonDsPath = "C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe"
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

$ArtifactFile = Get-Item -LiteralPath $ResolvedArtifactPath
Write-Output ("ARTIFACT=" + $ArtifactFile.FullName)
Write-Output ("ARTIFACT_LAST_WRITE_TIME=" + $ArtifactFile.LastWriteTime.ToString("O"))
Write-Output ("EMULATOR=" + $ResolvedMelonDsPath)

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
            exit 7
        }
    }

    $MelonDsProcess = Start-Process -FilePath $ResolvedMelonDsPath -ArgumentList @($ResolvedArtifactPath) -WorkingDirectory (Split-Path $ResolvedMelonDsPath) -PassThru
    Write-Output ("PROCESS_ID=" + $MelonDsProcess.Id)
} catch {
    [Console]::Error.WriteLine("Failed to launch melonDS: " + $_.Exception.Message)
    exit 8
}

exit 0
