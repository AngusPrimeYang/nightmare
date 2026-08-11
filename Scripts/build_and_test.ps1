#Requires -Version 5.1
<#
.SYNOPSIS
  Build the Unreal project and optionally run the Logic Gate (Automation RunTests).

.DESCRIPTION
  Loop-engineering dual gate:
    1) Compile via Build.bat (UE_ROOT from env / ue.local.env)
    2) Logic Gate via UnrealEditor-Cmd Automation RunTests when enabled

  Logic Gate runs when UE_RUN_TESTS is 1/true (env or ue.local.env) or -RunTests is passed,
  unless -SkipTests is set (compile only).

.PARAMETER Clean
  Pass -Clean to UBT before building.

.PARAMETER SkipTests
  Compile only; do not run Automation RunTests.

.PARAMETER RunTests
  Force Logic Gate after a successful build (overrides UE_RUN_TESTS=0).

.PARAMETER ExtraArgs
  Additional arguments forwarded to Build.bat.
#>
[CmdletBinding()]
param(
    [switch]$Clean,
    [switch]$SkipTests,
    [switch]$RunTests,
    [string[]]$ExtraArgs = @()
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

. (Join-Path $PSScriptRoot 'Resolve-UeEnv.ps1')
$envInfo = Resolve-UeEnv

if (-not (Test-Path -LiteralPath $envInfo.UProject)) {
    throw "Cannot build: missing .uproject at $($envInfo.UProject). Create the UE C++ project first."
}

function Test-UeTruthy {
    param([string]$Value)
    if ([string]::IsNullOrWhiteSpace($Value)) { return $false }
    switch ($Value.Trim().ToLowerInvariant()) {
        '1' { return $true }
        'true' { return $true }
        'yes' { return $true }
        'on' { return $true }
        default { return $false }
    }
}

$testFilter = $envInfo.UE_TEST_FILTER
if ([string]::IsNullOrWhiteSpace($testFilter)) {
    $testFilter = 'Nightmare.'
}

$shouldRunTests = $false
if ($SkipTests) {
    $shouldRunTests = $false
}
elseif ($RunTests) {
    $shouldRunTests = $true
}
else {
    $shouldRunTests = Test-UeTruthy -Value $envInfo.UE_RUN_TESTS
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
Write-Host "RunTests    : $shouldRunTests (filter=$testFilter)"
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
    $errorLines = @(
        Select-String -Path $logPath -Pattern 'error C[0-9]+|error LNK[0-9]+|Error:|:\s+error:' -AllMatches |
            ForEach-Object { $_.Line } |
            Select-Object -First 40
    )
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

if (-not $shouldRunTests) {
    Write-Host 'Logic Gate skipped (-SkipTests or UE_RUN_TESTS not enabled).'
    exit 0
}

$editorCmd = Join-Path $envInfo.UE_ROOT 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path -LiteralPath $editorCmd)) {
    throw "UnrealEditor-Cmd.exe not found: $editorCmd"
}

$testLogPath = Join-Path $logDir ("test_" + $stamp + '.log')
$testSummaryPath = Join-Path $logDir ("test_" + $stamp + '.summary.txt')
# RunTests is async; -testexit waits for the queue to drain before the process exits.
# Quote carefully: spaces in -testexit must stay one argv (Start-Process array joining is unreliable).
$execCmds = "Automation RunTests $testFilter"
$argLine = @(
    ('"' + $envInfo.UProject + '"'),
    '-unattended',
    '-NullRHI',
    '-nosound',
    '-nosplash',
    '-nop4',
    '-stdout',
    '-log',
    ('-abslog="' + $testLogPath + '"'),
    '-testexit="Automation Test Queue Empty"',
    ('-ExecCmds="' + $execCmds + '"')
) -join ' '

Write-Host "Logic Gate  : `"$editorCmd`" filter=$testFilter"
Write-Host "Test log    : $testLogPath"
Write-Host "Editor args : $argLine"

$testStart = Get-Date
$testProc = Start-Process -FilePath $editorCmd -ArgumentList $argLine -Wait -PassThru -NoNewWindow
$testElapsed = (Get-Date) - $testStart
$testExit = $testProc.ExitCode
if ($null -eq $testExit) {
    $testExit = 0
}

$failLines = @()
$passHints = @()
$ranHints = @()
if (Test-Path -LiteralPath $testLogPath) {
    $failLines = @(
        Select-String -Path $testLogPath -Pattern 'Result=\{Fail\}|Test Completed\. Result=\{Fail\}' |
            ForEach-Object { $_.Line } |
            Select-Object -First 40
    )
    $passHints = @(
        Select-String -Path $testLogPath -Pattern 'Result=\{Success\}|Test Completed\. Result=\{Success\}' |
            ForEach-Object { $_.Line } |
            Select-Object -First 40
    )
    $ranHints = @(
        Select-String -Path $testLogPath -Pattern 'RunTests|Found\s+\d+\s+automation|Queuing tests|Test Exit|Nightmare\.Smoke|Automation Test Queue Empty' |
            ForEach-Object { $_.Line } |
            Select-Object -First 40
    )
}

$logicOk = ($failLines.Count -eq 0) -and ($passHints.Count -gt 0)
if (-not $logicOk -and $failLines.Count -eq 0 -and $passHints.Count -eq 0) {
    # No parseable Success/Fail — treat as failure so the gate cannot silently pass.
    $logicOk = $false
}

$logicExit = 0
if (-not $logicOk) {
    $logicExit = 1
}
if ($testExit -ne 0 -and $logicExit -eq 0) {
    # Editor soft-fail: still surface process exit when we somehow saw Success.
    $logicExit = $testExit
}

@(
    "exit_code=$logicExit"
    "editor_exit=$testExit"
    "elapsed_sec=$([int]$testElapsed.TotalSeconds)"
    "ue_root=$($envInfo.UE_ROOT)"
    "uproject=$($envInfo.UProject)"
    "filter=$testFilter"
    "log=$testLogPath"
    "success_hits=$($passHints.Count)"
    "fail_hits=$($failLines.Count)"
    '--- fails (first 40) ---'
) + $failLines + @(
    '--- success hits (first 40) ---'
) + $passHints + @(
    '--- run hints (first 40) ---'
) + $ranHints | Set-Content -LiteralPath $testSummaryPath -Encoding UTF8

if ($logicExit -ne 0) {
    Write-Host "LOGIC GATE FAILED (exit $logicExit). Summary: $testSummaryPath"
    if ($failLines.Count -gt 0) {
        Write-Host '--- fail excerpt ---'
        $failLines | ForEach-Object { Write-Host $_ }
    }
    elseif ($passHints.Count -eq 0) {
        Write-Host 'No Success/Fail markers parsed from test log; see full log.'
    }
    exit $logicExit
}

Write-Host "LOGIC GATE SUCCEEDED in $([int]$testElapsed.TotalSeconds)s. Summary: $testSummaryPath"
exit 0
