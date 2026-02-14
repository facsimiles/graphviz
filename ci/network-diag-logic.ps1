# network-diag-logic.ps1: Try to diagnosis the network issues we see

Write-Host "Starting network-diag-logic.ps1"
Set-PSDebug -Trace 1

# List of alternate reliable mirrors to test
$altMirrors = @(
    "mirrors.sonstiges.net",
    "plug-mirror.rcac.purdue.edu",
    "ftp.eq.uc.pt"
)

Write-Host "!!! NETWORK DIAGNOSTICS REPORT !!!" -ForegroundColor Cyan

# 1. Test the Primary Target
$primary = "mirrors.kernel.org"
Write-Host "Testing Primary: $primary..."
$t1 = Test-NetConnection -ComputerName $primary -Port 443
# -InformationLevel Quiet
if ($t1) {
    Write-Host "  [OK] Primary is reachable." -ForegroundColor Green
} else {
    Write-Host "  [FAIL] Primary is unreachable." -ForegroundColor Red

    # 2. Test Alternates (Only if primary fails)
    Write-Host "Testing Alternate Mirrors..."
    foreach ($m in $altMirrors) {
        $t = Test-NetConnection -ComputerName $m -Port 443 -InformationLevel Quiet
        if ($t) {
            Write-Host "  [OK] $m is reachable." -ForegroundColor Green
        } else {
            Write-Host "  [FAIL] $m is unreachable." -ForegroundColor Red
        }
    }
}

# 3. DNS Resolution Check
Write-Host "`n--- DNS Status ---"
try {
    $dns = Resolve-DnsName -Name $primary -ErrorAction Stop
    Write-Host "  DNS Resolution for $primary: SUCCESS ($($dns.IPAddress))"
} catch {
    Write-Host "  DNS Resolution for $primary: FAILED" -ForegroundColor Red
    Get-DnsClientServerAddress | Select-Object InterfaceAlias, ServerAddresses | Format-Table -AutoSize | Out-String | Write-Host
}

# 4. Routing & Interface Stats (Did we lose the gateway?)
Write-Host "--- Routing & Interface ---"
Get-NetRoute -DestinationPrefix "0.0.0.0/0" | Select-Object DestinationPrefix, NextHop, RouteMetric | Format-Table -AutoSize | Out-String | Write-Host

# Check if the interface is actually sending/receiving packets
Get-NetAdapter | Get-NetAdapterStatistics | Select-Object Name, ReceivedBytes, SentBytes, ReceivedPacketErrors | Format-Table -AutoSize | Out-String | Write-Host
