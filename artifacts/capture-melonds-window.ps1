$source = @"
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public static class WindowCapture {
    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

    public struct RECT {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    public static void Save(IntPtr handle, string path) {
        RECT rect;
        if (!GetWindowRect(handle, out rect)) {
            throw new InvalidOperationException("GetWindowRect failed.");
        }

        int width = rect.Right - rect.Left;
        int height = rect.Bottom - rect.Top;
        using (Bitmap bitmap = new Bitmap(width, height)) {
            using (Graphics graphics = Graphics.FromImage(bitmap)) {
                graphics.CopyFromScreen(rect.Left, rect.Top, 0, 0, new Size(width, height));
            }

            bitmap.Save(path, ImageFormat.Png);
        }
    }
}
"@;

Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition $source;
$process = Get-Process melonDS | Select-Object -First 1;
if ($process -eq $null) {
    throw "melonDS is not running.";
}

[WindowCapture]::Save($process.MainWindowHandle, "C:\dev\helworks\helengine-ds\artifacts\melonds-window.png");
