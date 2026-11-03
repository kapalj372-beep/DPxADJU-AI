param([string]$Build = (Join-Path $PSScriptRoot '..\\build'), [string]$Dist = (Join-Path $PSScriptRoot '..\\dist-windows'))
$ErrorActionPreference = 'Stop'
$artifact = Join-Path $Build 'DPxADJU_AI_artefacts\\Release'
$vst3 = Get-ChildItem (Join-Path $artifact 'VST3') -Directory -Filter '*.vst3' -ErrorAction SilentlyContinue | Select-Object -First 1
$standalone = Get-ChildItem (Join-Path $artifact 'Standalone') -File -Filter '*.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
if (!$vst3) { throw "VST3 bundle not found under $artifact\\VST3. Build DPxADJU_AI_VST3 first." }
if (!$standalone) { throw "Standalone EXE not found under $artifact\\Standalone. Build DPxADJU_AI_Standalone first." }
New-Item -ItemType Directory -Force -Path $Dist | Out-Null
$targetVst3 = Join-Path $Dist 'DPxADJU AI.vst3'
if (Test-Path $targetVst3) { Remove-Item $targetVst3 -Recurse -Force }
Copy-Item $vst3.FullName $targetVst3 -Recurse
Copy-Item $standalone.FullName (Join-Path $Dist 'DPxADJU AI Studio.exe') -Force
Write-Host "Collected VST3 and Standalone files into $Dist"
