# Button/LED demo serial test for S32K344 core board
#
# Opens COM5 @ 115200 8N1, drains the boot banner, sends "disable"
# to stop the 10 ms counter stream, then captures raw RX bytes for
# the given number of seconds.  Press SW1 (PTA7) during the capture
# window: each confirmed press prints "SW1 pressed -> LED blink ...".
#
# Usage:
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools/test_button_led.ps1 -Seconds 45

param(
    [int]    $Seconds = 45,
    [string] $Port    = "COM5"
)

$baud = 115200

# Note: use ::new() + explicit enums — New-Object's parenthesized
# form is a parse error in PS 5.1, and the parameterless form falls
# back to a string when resolution fails.
$sp = [System.IO.Ports.SerialPort]::new(
    $Port, $baud,
    [System.IO.Ports.Parity]::None, 8,
    [System.IO.Ports.StopBits]::One)
$sp.ReadTimeout = 500

try {
    $sp.Open()
} catch {
    Write-Host ("ERROR: cannot open {0} - {1}" -f $Port, $_.Exception.Message)
    Write-Host "Close any serial monitor / debug session holding the port."
    exit 1
}

Start-Sleep -Milliseconds 300

# Drain the boot banner (and any 10 ms counter lines) for 1 s
$drainUntil = (Get-Date).AddMilliseconds(1000)
while ((Get-Date) -lt $drainUntil) {
    try {
        while ($sp.BytesToRead -gt 0) {
            $b = $sp.ReadByte()
            if ($b -ge 0) { [Console]::Write([char]$b) }
        }
    } catch {}
    Start-Sleep -Milliseconds 20
}

# Stop the 10 ms counter stream so button messages are readable
$sp.Write("disable`r")
Start-Sleep -Milliseconds 300

Write-Host ""
Write-Host "--- capture window: press SW1 now ---"
$end = (Get-Date).AddSeconds($Seconds)
$lastDisable = [DateTime]::Now
while ((Get-Date) -lt $end) {
    # Re-send "disable" every 3 s — a board reset during the window
    # re-enables the TX stream, this keeps it quiet again.
    if (([DateTime]::Now - $lastDisable).TotalSeconds -ge 3) {
        $sp.Write("disable`r")
        $lastDisable = [DateTime]::Now
    }
    try {
        while ($sp.BytesToRead -gt 0) {
            $b = $sp.ReadByte()
            if ($b -ge 0) { [Console]::Write([char]$b) }
        }
    } catch {}
    Start-Sleep -Milliseconds 10
}

$sp.Close()
Write-Host ""
Write-Host "--- capture end ---"
