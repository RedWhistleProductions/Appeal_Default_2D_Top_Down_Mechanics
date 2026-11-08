param([string]$AppealResources = '')
$ErrorActionPreference = 'Stop'
$Project = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
if (-not $AppealResources) {
    $AppealResources = (Resolve-Path (Join-Path $Project '../../..')).Path
}
& make -C $Project
if ($LASTEXITCODE -ne 0) { throw 'Default Mechanics build failed.' }
$Artifact = Join-Path $Project 'Appeal_Default_2D_Top_Down_Mechanics.dll'
$Destination = Join-Path $AppealResources 'Modules/Appeal_Module_Mechanics/Appeal_Default_2D_Top_Down_Mechanics'
New-Item -ItemType Directory -Path $Destination -Force | Out-Null
$Target = Join-Path $Destination 'Appeal_Default_2D_Top_Down_Mechanics.dll'
if ([IO.Path]::GetFullPath($Artifact) -ne [IO.Path]::GetFullPath($Target)) {
    Copy-Item -LiteralPath $Artifact -Destination $Target -Force
}
Write-Output "Installed $Target"
