param(
    [Parameter(Mandatory = $true)]
    [int]$RelativeX,

    [Parameter(Mandatory = $true)]
    [int]$RelativeY
)

$source = @"
using System;
using System.Runtime.InteropServices;

public static class WindowInput {
    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool SetCursorPos(int x, int y);

    [DllImport("user32.dll")]
    public static extern void mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, UIntPtr dwExtraInfo);

    public const uint MouseLeftDown = 0x0002;
    public const uint MouseLeftUp = 0x0004;

    public struct RECT {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }
}
"@;

Add-Type -TypeDefinition $source;
$process = Get-Process melonDS | Select-Object -First 1;
if ($process -eq $null) {
    throw "melonDS is not running.";
}

$rect = New-Object WindowInput+RECT;
[WindowInput]::GetWindowRect($process.MainWindowHandle, [ref]$rect) | Out-Null;
$x = $rect.Left + $RelativeX;
$y = $rect.Top + $RelativeY;
[WindowInput]::SetCursorPos($x, $y) | Out-Null;
Start-Sleep -Milliseconds 100;
[WindowInput]::mouse_event([WindowInput]::MouseLeftDown, 0, 0, 0, [UIntPtr]::Zero);
Start-Sleep -Milliseconds 50;
[WindowInput]::mouse_event([WindowInput]::MouseLeftUp, 0, 0, 0, [UIntPtr]::Zero);
