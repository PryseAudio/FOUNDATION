<#
.SYNOPSIS
    Configure, build and (optionally) test FOUNDATION on Windows.
.EXAMPLE
    ./10_BUILD/scripts/build.ps1 -Configuration Debug
    ./10_BUILD/scripts/build.ps1 -Configuration Release -SkipTests
#>
param(
    [ValidateSet('Debug', 'RelWithDebInfo', 'Release')]
    [string] $Configuration = 'Debug',
    [string] $ConfigurePreset = 'windows-vs2026',
    [switch] $SkipTests
)

$ErrorActionPreference = 'Stop'
$root = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$prefix = if ($ConfigurePreset -like 'ci-*') { 'ci-windows' } else { 'windows' }
$preset = "$prefix-$($Configuration.ToLower())"

Push-Location $root
try {
    cmake --preset $ConfigurePreset
    if ($LASTEXITCODE -ne 0) { throw "Configure failed ($LASTEXITCODE)" }

    cmake --build --preset $preset --parallel
    if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)" }

    if (-not $SkipTests) {
        ctest --preset $preset
        if ($LASTEXITCODE -ne 0) { throw "Tests failed ($LASTEXITCODE)" }
    }
}
finally {
    Pop-Location
}
