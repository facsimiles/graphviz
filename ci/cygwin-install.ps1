# cygwin-install.ps1: Prepare for running cygwin builds and tests

Write-Host "Starting cygwin-install.ps1"
$ProgressPreference = "SilentlyContinue"
Set-PSDebug -Trace 1

# Set-PSDebug -Trace 1
$Env:CI_COMMIT_DESCRIPTION = $null
$Env:CI_COMMIT_MESSAGE = $null
$Env:CI_COMMIT_TITLE = $null
# Set-PSDebug -Trace 1

# 1. HEALTH REPORT
Write-Host "--- Runner Health ---"
Get-PSDrive C | Select-Object @{N="FreeGB";E={[math]::round($_.Free/1GB,2)}} | Out-Host
$mem = Get-CimInstance Win32_OperatingSystem
Write-Host "Free Memory: $([math]::round($mem.FreePhysicalMemory/1MB, 2)) GB"

$cygwinPath = "C:\cygwin64"
$mirror = "https://mirrors.kernel.org/sourceware/cygwin"
$mirrorHost = "mirrors.kernel.org"
Write-Host "cygwinPath=$cygwinPath, mirror=$mirror, mirrorHost=$mirrorHost"

# 1. NETWORK CHECK
Write-Host "--- Testing NetConnection ---"
if (-not (Test-NetConnection $mirrorHost -Port 443).TcpTestSucceeded) {
    Write-Error "Mirror $mirrorHost is unreachable. Installation likely to fail."
}

# 2. CLEANUP (Services & Processes)
# Write-Host "--- Cleaning Up Existing Cygwin ---"
# Get-Service | Where-Object { $_.Status -eq 'Running' } | ForEach-Object {
#     $path = (Get-ItemProperty "HKLM:\SYSTEM\CurrentControlSet\Services\$($_.Name)").ImagePath
#    if ($path -like "*$cygwinPath*") { Stop-Service $_.Name -Force }
# }
# Get-Process | Where-Object { $_.Path -like "$cygwinPath*" } | Stop-Process -Force -EA 0

# 3. FETCH INSTALLER RETRY LOOP
Write-Host "--- Fetching Cygwin Installer ---"

$uri = "https://cygwin.com/setup-x86_64.exe"
$localfile = "$env:TEMP\setup-x86_64.exe"

$attempt = 1
$maxAttempts = 3

while ($attempt -le $maxAttempts) {
    Write-Host "Cygwin Setup Download Attempt $attempt..."
    try {
        $p = Invoke-WebRequest $uri -Verbose -Debug -OutFile $localfile
        Write-Host "Successful fetch of https://cygwin.com/setup-x86_64.exe to $localfile"
        $success = $true
        break
    }
    catch {
        if ($attempt -lt $maxAttempts) {
            Write-Warning "Attempt $attempt failed (Code: $($_.Exception.Message))..."
            Start-Sleep -Seconds 17
        } else {
            Write-Error "Attempt $attempt failed (Code: $($_.Exception.Message))."
        }
    }
    $attempt++
}

if (-not $success) {
    Write-Error "Cygwin installer download failed after $maxAttempts attempts."
    try {
        . "$PSScriptRoot/network-diag-logic.ps1"
    } catch {
       Write-Warning "Diagnostic script failed: $_"
    }
    exit 1
}

$Env:CYGWIN_SETUP = $localfile

# 4. INSTALL CYGWIN RETRY LOOP
Write-Host "--- Running Cygwin Installer ---"
$success = $false
for ($i=1; $i -le 3; $i++) {
    Write-Host "Cygwin Setup Run Attempt $i..."
    $p = Start-Process "$localfile" -ArgumentList "--quiet-mode --site $mirror --wait" -Wait -PassThru
    if ($p.ExitCode -eq 0) { $success = $true; break }
    if ($i -lt 3) {
        Write-Warning "Cygwin Setup Run attempt $i failed (Code: $($p.ExitCode)). Sleeping 15s..."
        copy C:\cygwin64\var\log\setup.log.full ".\setup-full-$i.log"
        Start-Sleep -Seconds 15
        Write-Warning "Finished sleeping"
    } else {
        Write-Error "Cygwin Setup Run attempt $i failed (Code: $($p.ExitCode))."
        gci env:
    }
}

if (-not $success) {
    Write-Error "Cygwin installation failed after 3 attempts."
    try {
        . "$PSScriptRoot/network-diag-logic.ps1"
    } catch {
       Write-Warning "Diagnostic script failed: $_"
    }
    exit 1
}
Remove-Item -Path 'C:\Python313' -Recurse -Force

$Env:Path = "C:\cygwin64\bin;" + $Env:Path

# fix CRLF in scripts we will run with cygwin
C:\cygwin64\bin\find . -type f '(' -name Makefile.am -o -name "*.def" -o -name '*.sh' -o -name '*.ac' ')' -exec C:\cygwin64\bin\sed -i 's/\r//g' "{}" ';'
C:\cygwin64\bin\sed -i 's/\r//g' autogen.sh ci/*.sh lib/common/color_names lib/common/brewer_colors lib/common/svgcolor_names

$Env:build_system = "autotools"
C:\cygwin64\bin\bash -l -c 'cd $CI_PROJECT_DIR && ci/cygwin-build.sh'

$Env:Path = "C:\cygwin64\bin;" + $Env:Path
