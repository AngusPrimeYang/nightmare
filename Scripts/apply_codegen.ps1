#Requires -Version 5.1
<#
.SYNOPSIS
  Apply complete codegen files (full .h/.cpp overwrite) into the Unreal Source tree.

.DESCRIPTION
  Loop-engineering apply step. Rejects patch/diff fragments. Destinations are resolved
  under <RepoRoot>/Source/ unless -DestinationRoot is provided.

.PARAMETER SourcePath
  File or directory containing complete source files to apply.

.PARAMETER DestinationRoot
  Optional override root (default: <RepoRoot>/Source).

.PARAMETER DryRun
  Show planned copies without writing.

.PARAMETER Force
  Overwrite existing files (default behavior is overwrite; kept for clarity).
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$SourcePath,

    [string]$DestinationRoot = '',

    [switch]$DryRun,

    [switch]$Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

. (Join-Path $PSScriptRoot 'Resolve-UeEnv.ps1')
$envInfo = Resolve-UeEnv

if ([string]::IsNullOrWhiteSpace($DestinationRoot)) {
    $DestinationRoot = Join-Path $envInfo.RepoRoot 'Source'
}

$resolvedSource = (Resolve-Path -LiteralPath $SourcePath).Path
if (-not (Test-Path -LiteralPath $DestinationRoot)) {
    if ($DryRun) {
        Write-Warning "DestinationRoot does not exist yet (will be created on real run): $DestinationRoot"
    }
    else {
        New-Item -ItemType Directory -Force -Path $DestinationRoot | Out-Null
    }
}

$files = @()
if (Test-Path -LiteralPath $resolvedSource -PathType Leaf) {
    $files = @(Get-Item -LiteralPath $resolvedSource)
}
else {
    $files = @(Get-ChildItem -LiteralPath $resolvedSource -Recurse -File -Include *.h, *.hpp, *.cpp, *.c, *.cs, *.inl)
}

if ($files.Count -eq 0) {
    throw "No source files found under: $resolvedSource"
}

$patchMarkers = @(
    '@@ ',
    'diff --git ',
    '*** Begin Patch',
    '*** Update File:',
    '--- a/',
    '+++ b/'
)

$applied = @()
$rejected = @()

foreach ($file in $files) {
    $text = Get-Content -LiteralPath $file.FullName -Raw -Encoding UTF8
    if ([string]::IsNullOrWhiteSpace($text)) {
        $rejected += [pscustomobject]@{ Path = $file.FullName; Reason = 'empty file' }
        continue
    }

    $isPatch = $false
    foreach ($marker in $patchMarkers) {
        if ($text.Contains($marker)) {
            $isPatch = $true
            break
        }
    }
    if ($isPatch) {
        $rejected += [pscustomobject]@{ Path = $file.FullName; Reason = 'looks like a patch/diff fragment (full file required)' }
        continue
    }

    # Relative path: if SourcePath is a directory, preserve subtree; if a file, use file name only
    # unless the path already contains Source/<Module>/...
    $relative = $null
    if (Test-Path -LiteralPath $resolvedSource -PathType Container) {
        $relative = $file.FullName.Substring($resolvedSource.Length).TrimStart('\', '/')
    }
    else {
        $relative = $file.Name
        # Allow staging layouts like .../Source/Nightmare/Foo.h
        $idx = $file.FullName.Replace('/', '\').ToLowerInvariant().LastIndexOf('\source\')
        if ($idx -ge 0) {
            $relative = $file.FullName.Substring($idx + '\source\'.Length)
        }
    }

    $dest = Join-Path $DestinationRoot $relative
    $destDir = Split-Path -Parent $dest

    if ($DryRun) {
        Write-Host "[DryRun] $($file.FullName) -> $dest"
    }
    else {
        if (-not (Test-Path -LiteralPath $destDir)) {
            New-Item -ItemType Directory -Force -Path $destDir | Out-Null
        }
        Copy-Item -LiteralPath $file.FullName -Destination $dest -Force
        Write-Host "Applied: $dest"
    }

    $applied += [pscustomobject]@{ Source = $file.FullName; Destination = $dest }
}

Write-Host ""
Write-Host "Applied : $($applied.Count)"
Write-Host "Rejected: $($rejected.Count)"

if ($rejected.Count -gt 0) {
    Write-Host '--- rejected ---'
    $rejected | ForEach-Object { Write-Host ("{0} :: {1}" -f $_.Path, $_.Reason) }
    throw 'One or more files were rejected. Provide complete .h/.cpp files only (loop engineering rule #4).'
}

if (-not $DryRun -and $applied.Count -gt 0) {
    $logDir = Join-Path $envInfo.RepoRoot 'Saved\LoopEngineering'
    New-Item -ItemType Directory -Force -Path $logDir | Out-Null
    $stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
    $manifest = Join-Path $logDir ("apply_" + $stamp + '.manifest.txt')
    $applied | ForEach-Object { "{0}`t->{1}" -f $_.Source, $_.Destination } |
        Set-Content -LiteralPath $manifest -Encoding UTF8
    Write-Host "Manifest: $manifest"
}

exit 0
