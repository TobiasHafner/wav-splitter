$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Resolve-Path "$ScriptDir/../.."

& "$ProjectRoot\scripts\windows\clean.ps1"
& "$ProjectRoot\scripts\windows\build.ps1"

