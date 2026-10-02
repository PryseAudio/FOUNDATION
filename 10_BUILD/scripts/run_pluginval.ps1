<#
.SYNOPSIS
    Validate the built FOUNDATION VST3 with pluginval (strictness 10).
.DESCRIPTION
    pluginval is located via -Pluginval, $env:FOUNDATION_PLUGINVAL, PATH, or 10_BUILD/tools/pluginval.exe.
    Exit code 0 = passed, 1 = failed, 2 = pluginval not available.
#>
param(
    [ValidateSet('Debug', 'RelWithDebInfo', 'Release')]
    [string] $Configuration = 'Release',
    [string] $ConfigurePreset = 'windows-vs2026',
    [string] $Pluginval = '',
    [int] $Strictness = 10
)

$ErrorActionPreference = 'Stop'
$root = Resolve-Path (Join-Path $PSScriptRoot '..\..')

$candidates = @($Pluginval, $env:FOUNDATION_PLUGINVAL)
$onPath = Get-Command pluginval -ErrorAction SilentlyContinue
if ($onPath) { $candidates += $onPath.Source }
$candidates += (Join-Path $root '10_BUILD\tools\pluginval.exe')

$exe = $candidates | Where-Object { $_ -and (Test-Path $_) } | Select-Object -First 1
if (-not $exe) {
    Write-Warning 'pluginval not found (set FOUNDATION_PLUGINVAL or place it in 10_BUILD/tools/).'
    exit 2
}

$buildDir = Join-Path (Split-Path $root -Parent) "FOUNDATION_build\$ConfigurePreset"
$plugin = Join-Path $buildDir "11_PLUGIN\Foundation_artefacts\$Configuration\VST3\FOUNDATION.vst3"
if (-not (Test-Path $plugin)) { throw "VST3 not found: $plugin" }

$reportDir = Join-Path $buildDir "pluginval\$Configuration"
New-Item -ItemType Directory -Force $reportDir | Out-Null

Write-Host "pluginval: $exe"
Write-Host "plugin:    $plugin"

# pluginval.exe is a GUI-subsystem program on Windows: invoking it with '&' neither waits for it nor
# captures its output or exit code. Start-Process -Wait does both.
$stdout = Join-Path $reportDir 'stdout.txt'
$stderr = Join-Path $reportDir 'stderr.txt'
$arguments = @('--strictness-level', "$Strictness", '--validate-in-process',
               '--output-dir', "`"$reportDir`"", '--validate', "`"$plugin`"")
$process = Start-Process -FilePath $exe -ArgumentList $arguments -Wait -PassThru -NoNewWindow `
                         -RedirectStandardOutput $stdout -RedirectStandardError $stderr

Get-Content $stdout
Get-Content $stderr | ForEach-Object { Write-Warning $_ }

$code = $process.ExitCode
Write-Host "pluginval exit code: $code"
Write-Host "reports:   $reportDir"
if ($code -ne 0) { exit 1 }
exit 0
