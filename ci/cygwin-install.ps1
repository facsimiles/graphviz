# cygwin-install.ps1: Prepare for running cygwin builds and tests

Write-Host "Starting cygwin-install.ps1"

Set-PSDebug -Trace 1
$Env:CI_COMMIT_DESCRIPTION = $null
$Env:CI_COMMIT_MESSAGE = $null
$Env:CI_COMMIT_TITLE = $null
Set-PSDebug -Trace 1

# 1. HEALTH REPORT
Write-Host "--- Runner Health ---"
Get-PSDrive C | Select-Object @{N="FreeGB";E={[math]::round($_.Free/1GB,2)}} | Out-Host
$mem = Get-CimInstance Win32_OperatingSystem
Write-Host "Free Memory: $([math]::round($mem.FreePhysicalMemory/1MB, 2)) GB"
# 2. disable Windows Defender
Add-MpPreference -ExclusionPath 'C:\'

$cygwinPath = "C:\cygwin64"
$mirror = "https://mirrors.kernel.org"
$mirrorHost = "mirrors.kernel.org"

# 1. NETWORK CHECK
if (-not (Test-NetConnection $mirrorHost -Port 443).TcpTestSucceeded) {
    Write-Error "Mirror $mirrorHost is unreachable. Installation likely to fail."
}

# 2. CLEANUP (Services & Processes)
Write-Host "--- Cleaning Up Existing Cygwin ---"
Get-Service | Where-Object { $_.Status -eq 'Running' } | ForEach-Object {
    $path = (Get-ItemProperty "HKLM:\SYSTEM\CurrentControlSet\Services\$($_.Name)").ImagePath
    if ($path -like "*$cygwinPath*") { Stop-Service $_.Name -Force }
}
Get-Process | Where-Object { $_.Path -like "$cygwinPath*" } | Stop-Process -Force -EA 0

# 3. FETCH INSTALLER RETRY LOOP
$success = $false
for ($i=1; $i -le 3; $i++) {
    Write-Host "Cygwin Setup Attempt $i..."
    wget https://cygwin.com/setup-x86_64.exe -OutFile C:\setup-x86_64.exe
    if ($p.ExitCode -eq 0) { $success = $true; break }
    Write-Warning "Attempt $i failed (Code: $($p.ExitCode)). Sleeping 7s..."
    Start-Sleep -Seconds 7
}

if (-not $success) {
    Write-Error "Cygwin installater download failed after 3 attempts."
    $global::diag_needed = $true
}

# 4. INSTALL CYGWIN RETRY LOOP
$success = $false
for ($i=1; $i -le 3; $i++) {
    Write-Host "Cygwin Setup Attempt $i..."
    $p = Start-Process "C:\setup-x86_64.exe" -ArgumentList "--quiet-mode --site $mirror --wait" -Wait -PassThru
    if ($p.ExitCode -eq 0) { $success = $true; break }
    Write-Warning "Attempt $i failed (Code: $($p.ExitCode)). Sleeping 15s..."
    Start-Sleep -Seconds 15
}

if (-not $success) {
    Write-Error "Cygwin installation failed after 3 attempts."
    $global::diag_needed = $true
}
if ($global:diag_needed) {
    try {
        . "$PSScriptRoot/network-diag-logic.ps1"
    } catch {
       Write-Warning "Diagnostic script failed: $_"
    }
    exit 1
}
Remove-Item -Path 'C:\Python313' -Recurse -Force

$Env:Path = "C:\cygwin64\bin;" + $Env:Path
$Env:CFLAGS = "-Werror -Wno-error=implicit-fallthrough"
$Env:CXXFLAGS = "-Werror"

C:\cygwin64\bin\find . '(' -name Makefile.am -or -name "*.def" ')' -exec C:\cygwin64\bin\sed -i 's/\r//g' "{}" ';'
C:\cygwin64\bin\sed -i 's/\r//g' autogen.sh ci/*.sh configure.ac lib/common/color_names lib/common/brewer_colors lib/common/svgcolor_names

$Env:build_system = "autotools"
C:\cygwin64\bin\bash -l -c 'cd $CI_PROJECT_DIR && ci/cygwin-build.sh'

$Env:Path = "C:\cygwin64\bin;" + $Env:Path
