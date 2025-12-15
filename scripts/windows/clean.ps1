$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path "$ScriptDir/../.."

$BuildDir = Join-Path $ProjectRoot "build"
$DistDir  = Join-Path $ProjectRoot "dist"

Write-Host "Cleaning build directory..."

if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}

Write-Host "Cleaning dist directory..."

if (Test-Path $DistDir) {
    Remove-Item -Recurse -Force $BuildDir
}

Write-Host "Clean completed."

