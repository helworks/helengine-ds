[CmdletBinding()]
param(
    [Parameter()]
    [string]$RomPath = "",

    [Parameter()]
    [string]$MelonDsPath = "C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RomPath)) {
    $RomPath = Join-Path $PSScriptRoot "..\build\helengine_ds.nds"
}

if ([string]::IsNullOrWhiteSpace($MelonDsPath)) {
    [Console]::Error.WriteLine("MelonDsPath is required.")
    exit 2
}

$ResolvedRomPath = [System.IO.Path]::GetFullPath($RomPath)
if (-not (Test-Path -LiteralPath $ResolvedRomPath -PathType Leaf)) {
    [Console]::Error.WriteLine("ROM file was not found at '$ResolvedRomPath'.")
    exit 3
}

$ResolvedMelonDsPath = [System.IO.Path]::GetFullPath($MelonDsPath)
if (-not (Test-Path -LiteralPath $ResolvedMelonDsPath -PathType Leaf)) {
    [Console]::Error.WriteLine("melonDS executable was not found at '$ResolvedMelonDsPath'.")
    exit 4
}

$RomFile = Get-Item -LiteralPath $ResolvedRomPath
Write-Host ("ROM: " + $RomFile.FullName)
Write-Host ("ROM LastWriteTime: " + $RomFile.LastWriteTime.ToString("yyyy-MM-dd HH:mm:ss"))
Write-Host ("Launching: " + $ResolvedMelonDsPath)

try {
    $MelonDsProcess = Start-Process -FilePath $ResolvedMelonDsPath -ArgumentList @($ResolvedRomPath) -WorkingDirectory (Split-Path $ResolvedMelonDsPath) -PassThru
    Write-Host ("melonDS PID: " + $MelonDsProcess.Id)
} catch {
    [Console]::Error.WriteLine("Failed to launch melonDS: " + $_.Exception.Message)
    exit 5
}

exit 0
