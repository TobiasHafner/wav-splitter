$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path "$ScriptDir/../.."

$PackageName = "wav-splitter"
$BuildDir = Join-Path $ProjectRoot "build"
$DistDir  = Join-Path $ProjectRoot "dist"

# Version from git
try {
    $Version = git -C $ProjectRoot describe --tags --dirty --always
} catch {
    $Version = "v0.0.0"
}

Write-Host "Creating Windows release package for $PackageName ($Version)..."

# Build first
& "$ProjectRoot\scripts\windows\cleanbuild.ps1"

# Prepare output
if (Test-Path $DistDir) {
    Remove-Item -Recurse -Force $DistDir
}
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

$ZipName = "$PackageName-$Version-windows-x64.zip"
$ZipPath = Join-Path $DistDir $ZipName

# Collect files
$Staging = Join-Path $DistDir "staging"
New-Item -ItemType Directory -Force -Path $Staging | Out-Null

Copy-Item "$BuildDir\$PackageName.exe" $Staging
Copy-Item "$ProjectRoot\README.md" $Staging -ErrorAction SilentlyContinue
Copy-Item "$ProjectRoot\LICENSE" $Staging -ErrorAction SilentlyContinue

# Create ZIP
Compress-Archive -Path "$Staging\*" -DestinationPath $ZipPath

# Cleanup
Remove-Item -Recurse -Force $Staging

Write-Host "Windows release package created:"
Write-Host "  $ZipPath"

