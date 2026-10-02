<#
.SYNOPSIS
    Runs the FOUNDATION CI pipeline locally on Windows:
    configure -> build Debug + Release -> ctest both -> pluginval (Release) if available.
#>
param(
    [string] $ConfigurePreset = 'windows-vs2026'
)

$ErrorActionPreference = 'Stop'
$scripts = $PSScriptRoot

foreach ($configuration in 'Debug', 'Release') {
    Write-Host "=== $configuration ===" -ForegroundColor Cyan
    & (Join-Path $scripts 'build.ps1') -Configuration $configuration -ConfigurePreset $ConfigurePreset
}

Write-Host '=== pluginval (Release) ===' -ForegroundColor Cyan
& (Join-Path $scripts 'run_pluginval.ps1') -Configuration Release -ConfigurePreset $ConfigurePreset
switch ($LASTEXITCODE) {
    0 { Write-Host 'pluginval: PASSED' -ForegroundColor Green }
    2 { Write-Warning 'pluginval: SKIPPED (not available)' }
    default { throw 'pluginval: FAILED' }
}
