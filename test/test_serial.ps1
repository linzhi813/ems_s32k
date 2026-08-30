# ============================================================================
# test_serial.ps1 — 串口功能测试脚本（EMS S32K UART console demo）
#
# 验证内容：
#   1. 每 10 ms 周期发送 g_counter_10ms 的值（2 s 内应收到约 200 行，
#      且计数值逐行 +1）
#   2. 发送 "disable" 后停止发送（1 s 内不应再有新行）
#   3. 发送 "enable" 后恢复发送（1 s 内应再次收到约 100 行）
#
# 用法：
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools/test_serial.ps1 [-Port COM5] [-Baud 115200]
# ============================================================================

param(
    [string]$Port = "COM5",
    [int]$Baud = 115200
)

$ErrorActionPreference = "Stop"

# ── 打开串口 ────────────────────────────────────────────────────────────
$sp = New-Object System.IO.Ports.SerialPort $Port, $Baud, None, 8, One
$sp.ReadTimeout  = 200    # ms — ReadLine 超时即返回 null
$sp.WriteTimeout = 500
try {
    $sp.Open()
} catch {
    Write-Output "FAIL: cannot open $Port — $($_.Exception.Message)"
    exit 1
}
Start-Sleep -Milliseconds 300
$sp.DiscardInBuffer()

function Collect-Lines([double]$Seconds) {
    $lines = @()
    $t0 = [DateTime]::Now
    while (([DateTime]::Now - $t0).TotalSeconds -lt $Seconds) {
        try {
            $line = $sp.ReadLine()
            if ($null -ne $line -and $line.Length -gt 0) { $lines += $line }
        } catch [System.TimeoutException] {
            # no data — keep waiting
        }
    }
    return ,$lines
}

# ── Phase 1: 周期发送 ────────────────────────────────────────────────────
$p1 = Collect-Lines 2.0
Write-Output "PHASE1 (2 s): received $($p1.Count) lines"
foreach ($l in $p1 | Select-Object -First 3) { Write-Output "  sample: $l" }

# 检查行内容与 +1 递增（跳过首行，可能从半行开始）
$regex  = [regex]"^g_counter_10ms = (\d+)\s*$"
$gaps   = 0
$checks = 0
$prev   = -1
$ok1    = $false
foreach ($l in $p1) {
    $m = $regex.Match($l)
    if (-not $m.Success) { continue }
    $v = [uint32]$m.Groups[1].Value
    if ($prev -ne -1) {
        $checks++
        if ($v -ne ($prev + 1)) { $gaps++ }
    }
    $prev = $v
}
$ok1 = ($p1.Count -ge 120) -and ($checks -ge 50) -and ($gaps -eq 0)
Write-Output "PHASE1 check: lines=$($p1.Count) increments=$checks gaps=$gaps => $(if ($ok1) {'OK'} else {'FAIL'})"

# ── Phase 2: disable ─────────────────────────────────────────────────────
$sp.WriteLine("disable")
Start-Sleep -Milliseconds 300   # 等指令被处理
$sp.DiscardInBuffer()
$p2 = Collect-Lines 1.0
$ok2 = ($p2.Count -le 1)        # 允许 0 行（"TX disabled" 回执已被丢弃）
Write-Output "PHASE2 (after 'disable', 1 s): received $($p2.Count) lines => $(if ($ok2) {'OK'} else {'FAIL'})"
if ($p2.Count -gt 0) { foreach ($l in $p2) { Write-Output "  unexpected: $l" } }

# ── Phase 3: enable ──────────────────────────────────────────────────────
$sp.WriteLine("enable")
Start-Sleep -Milliseconds 200
$sp.DiscardInBuffer()
$p3 = Collect-Lines 1.0
Write-Output "PHASE3 (after 'enable', 1 s): received $($p3.Count) lines"
foreach ($l in $p3 | Select-Object -First 3) { Write-Output "  sample: $l" }
$ok3 = ($p3.Count -ge 60)

# 验证 enable 后计数器相对 phase1 末值连续
$m = $regex.Match($p3[0])
$cont = $false
if ($m.Success -and $prev -ne -1) {
    $v = [uint32]$m.Groups[1].Value
    $cont = ($v -gt $prev) -and (($v - $prev) -lt 400)   # 允许因关停时间造成的差值
    Write-Output "  counter continuity: last=$prev first_after_enable=$v => $(if ($cont) {'OK'} else {'FAIL'})"
}

$sp.Close()

Write-Output ""
$overall = $ok1 -and $ok2 -and $ok3
if ($overall) { Write-Output "RESULT: PASS" } else { Write-Output "RESULT: FAIL" }
exit $(if ($overall) { 0 } else { 1 })
