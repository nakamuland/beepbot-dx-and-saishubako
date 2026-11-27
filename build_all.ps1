$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Dist = Join-Path $Root 'dist'
New-Item -ItemType Directory -Force -Path $Dist | Out-Null

function Invoke-PioBuild([string]$Project, [string]$Environment, [string]$OutputName) {
    Push-Location $Project
    try {
        & python -m platformio run -e $Environment
        if ($LASTEXITCODE -ne 0) { throw "PlatformIO build failed in $Project" }
    } finally {
        Pop-Location
    }
    $Firmware = Join-Path $Project ".pio\build\$Environment\firmware.bin"
    if (-not (Test-Path -LiteralPath $Firmware)) { throw "Missing firmware: $Firmware" }
    Copy-Item -LiteralPath $Firmware -Destination (Join-Path $Dist $OutputName) -Force
}

Invoke-PioBuild (Join-Path $Root 'beepbotdx') 'm5stack-cardputer' 'beepbotdx-k132-ota1.bin'
Invoke-PioBuild (Join-Path $Root 'saishubako') 'm5stack-cardputer' 'saishubako-k132-ota1.bin'
& python (Join-Path $Root 'tools\embed_app_images.py') `
    --beepbot (Join-Path $Dist 'beepbotdx-k132-ota1.bin') `
    --saishubako (Join-Path $Dist 'saishubako-k132-ota1.bin') `
    --output (Join-Path $Root 'launcher\src\embedded_apps.S')
if ($LASTEXITCODE -ne 0) { throw 'Embedding source generation failed.' }
Invoke-PioBuild (Join-Path $Root 'launcher') 'launcher' 'k132-app-launcher-ota0.bin'
Invoke-PioBuild (Join-Path $Root 'unitcams3') 'unitcams3' 'unitcams3-camera.bin'
& python (Join-Path $Root 'tools\verify_artifacts.py')
if ($LASTEXITCODE -ne 0) { throw 'Artifact verification failed.' }
$Names = @('beepbotdx-k132-ota1.bin', 'saishubako-k132-ota1.bin', 'k132-app-launcher-ota0.bin', 'unitcams3-camera.bin')
$Hashes = foreach ($Name in $Names) {
    $Hash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $Dist $Name)).Hash.ToLowerInvariant()
    "$Hash  $Name"
}
$Hashes | Set-Content -Encoding ascii (Join-Path $Dist 'SHA256SUMS.txt')
Write-Host "Build complete. No device or microSD operations were performed. Artifacts: $Dist"
