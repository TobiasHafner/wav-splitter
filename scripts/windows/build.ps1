$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path "$ScriptDir/../.."
$BuildDir = Join-Path $ProjectRoot "build"

Write-Host "Building wav-splitter..."
Write-Host "Build directory: $BuildDir"

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
Set-Location $BuildDir

cmake $ProjectRoot -G Ninja
ninja

Write-Host "Build completed."

