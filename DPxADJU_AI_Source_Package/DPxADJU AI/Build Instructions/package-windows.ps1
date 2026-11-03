param(
  [string]$Dist = (Join-Path $PSScriptRoot '..\\dist-windows'),
  [string]$Nsi = (Join-Path $PSScriptRoot '..\\Installer\\DPxADJU-AI.nsi')
)
$ErrorActionPreference = 'Stop'
$Vst3 = Join-Path $Dist 'DPxADJU AI.vst3'
$Exe = Join-Path $Dist 'DPxADJU AI Studio.exe'
if (!(Test-Path $Vst3 -PathType Container)) { throw "Missing VST3 bundle: $Vst3. Build the JUCE VST3 target first." }
if (!(Test-Path $Exe -PathType Leaf)) { throw "Missing Standalone EXE: $Exe. Build the JUCE Standalone target first." }
$Makensis = Get-Command makensis -ErrorAction SilentlyContinue
if (!$Makensis) { throw 'NSIS (makensis.exe) is required to generate Installer.exe.' }
$env:DPX_DIST = (Resolve-Path $Dist).Path
Push-Location (Split-Path $Nsi -Parent)
try { & $Makensis.Source "/DDPX_DIST=$($env:DPX_DIST)" (Split-Path $Nsi -Leaf); if ($LASTEXITCODE -ne 0) { throw "makensis failed: $LASTEXITCODE" } }
finally { Pop-Location }
Write-Host 'Installer generated next to the NSIS script.'
