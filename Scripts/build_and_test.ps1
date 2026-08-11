#Requires -Version 5.1
<#
.SYNOPSIS
  Build the Unreal project using externally configured UE_ROOT and report results.

.DESCRIPTION
  Loop-engineering compile gate. Reads UE_ROOT / UE_PROJECT / UE_TARGET / UE_PLATFORM / UE_CONFIG
  from process env or ue.local.env (see ue.local.env.example).

.PARAMETER Clean
  Pass -Clean to UBT before building.

.PARAMETER ExtraArgs
  Additional arguments forwarded to Build.bat.
#>
[CmdletBinding()]
param(
    [switch]$Clean,
    [string[]]$ExtraArgs = @()
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

. (Join-Path $PSScriptRoot 'Resolve-UeEnv.ps1')
$envInfo = Resolve-UeEnv

if (-not (Test-Path -LiteralPath $envInfo.UProject)) {
    throw "Cannot build: missing .uproject at $($envInfo.UProject). Create the UE C++ project first."
}

$logDir = Join-Path $envInfo.RepoRoot 'Saved\LoopEngineering'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
$logPath = Join-Path $logDir ("build_" + $stamp + '.log')

$ubtArgs = @(
    $envInfo.UE_TARGET,
    $envInfo.UE_PLATFORM,
    $envInfo.UE_CONFIG,
    ('-Project=' + $envInfo.UProject),
    '-WaitMutex',
    '-FromMsBuild'
)
if ($Clean) {
    $ubtArgs += '-Clean'
}
if ($ExtraArgs -and $ExtraArgs.Count -gt 0) {
    $ubtArgs += $ExtraArgs
}

Write-Host "UE_ROOT     : $($envInfo.UE_ROOT)"
Write-Host "UProject    : $($envInfo.UProject)"
Write-Host "Target      : $($envInfo.UE_TARGET) | $($envInfo.UE_PLATFORM) | $($envInfo.UE_CONFIG)"
Write-Host "Log         : $logPath"
Write-Host "Command     : `"$($envInfo.BuildBat)`" $($ubtArgs -join ' ')"

$start = Get-Date
& $envInfo.BuildBat @ubtArgs 2>&1 | Tee-Object -FilePath $logPath
$exitCode = $LASTEXITCODE
$elapsed = (Get-Date) - $start

if ($null -eq $exitCode) {
    $exitCode = 0
}

$summaryPath = Join-Path $logDir ("build_" + $stamp + '.summary.txt')
$errorLines = @()
if (Test-Path -LiteralPath $logPath) {
    $errorLines = Select-String -Path $logPath -Pattern 'error C[0-9]+|error LNK[0-9]+|Error:|:\s+error:' -AllMatches |
        ForEach-Object { $_.Line } |
        Select-Object -First 40
}

@(
    "exit_code=$exitCode"
    "elapsed_sec=$([int]$elapsed.TotalSeconds)"
    "ue_root=$($envInfo.UE_ROOT)"
    "uproject=$($envInfo.UProject)"
    "target=$($envInfo.UE_TARGET)"
    "platform=$($envInfo.UE_PLATFORM)"
    "config=$($envInfo.UE_CONFIG)"
    "log=$logPath"
    '--- errors (first 40) ---'
) + $errorLines | Set-Content -LiteralPath $summaryPath -Encoding UTF8

if ($exitCode -ne 0) {
    Write-Host "BUILD FAILED (exit $exitCode). Summary: $summaryPath"
    if ($errorLines.Count -gt 0) {
        Write-Host '--- error excerpt ---'
        $errorLines | ForEach-Object { Write-Host $_ }
    }
    exit $exitCode
}

Write-Host "BUILD SUCCEEDED in $([int]$elapsed.TotalSeconds)s. Summary: $summaryPath"
exit 0
