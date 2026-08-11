#Requires -Version 5.1
<#
.SYNOPSIS
  Resolve Unreal Engine paths and build settings from external parameters.

.DESCRIPTION
  Load order (later wins for empty-only fill; env vars always win over file):
    1. ue.local.env at repo root (KEY=VALUE lines)
    2. Process environment variables: UE_ROOT, UE_PROJECT, UE_TARGET, UE_PLATFORM, UE_CONFIG

  Machine-specific ue.local.env is gitignored. Commit ue.local.env.example as the template.
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-RepoRoot {
    return (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
}

function Import-DotEnvFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        return @{}
    }

    $map = @{}
    Get-Content -LiteralPath $Path -Encoding UTF8 | ForEach-Object {
        $line = $_.Trim()
        if ([string]::IsNullOrWhiteSpace($line)) { return }
        if ($line.StartsWith('#')) { return }

        $eq = $line.IndexOf('=')
        if ($eq -lt 1) { return }

        $key = $line.Substring(0, $eq).Trim()
        $value = $line.Substring($eq + 1).Trim()
        if (
            ($value.StartsWith('"') -and $value.EndsWith('"')) -or
            ($value.StartsWith("'") -and $value.EndsWith("'"))
        ) {
            $value = $value.Substring(1, $value.Length - 2)
        }
        $map[$key] = $value
    }
    return $map
}

function Get-UeEnvValue {
    param(
        [Parameter(Mandatory = $true)]
        [hashtable]$FileMap,
        [Parameter(Mandatory = $true)]
        [string]$Name,
        [string]$Default = ''
    )

    $fromEnv = [Environment]::GetEnvironmentVariable($Name)
    if (-not [string]::IsNullOrWhiteSpace($fromEnv)) {
        return $fromEnv.Trim()
    }
    if ($FileMap.ContainsKey($Name) -and -not [string]::IsNullOrWhiteSpace([string]$FileMap[$Name])) {
        return ([string]$FileMap[$Name]).Trim()
    }
    return $Default
}

function Resolve-UeEnv {
    [CmdletBinding()]
    param()

    $repoRoot = Get-RepoRoot
    $envFile = Join-Path $repoRoot 'ue.local.env'
    $exampleFile = Join-Path $repoRoot 'ue.local.env.example'
    $fileMap = Import-DotEnvFile -Path $envFile

    $ueRoot = Get-UeEnvValue -FileMap $fileMap -Name 'UE_ROOT'
    $ueProject = Get-UeEnvValue -FileMap $fileMap -Name 'UE_PROJECT' -Default 'Nightmare'
    $ueTarget = Get-UeEnvValue -FileMap $fileMap -Name 'UE_TARGET' -Default ($ueProject + 'Editor')
    $uePlatform = Get-UeEnvValue -FileMap $fileMap -Name 'UE_PLATFORM' -Default 'Win64'
    $ueConfig = Get-UeEnvValue -FileMap $fileMap -Name 'UE_CONFIG' -Default 'Development'

    if ([string]::IsNullOrWhiteSpace($ueRoot)) {
        throw @"
UE_ROOT is not set.

On this machine, either:
  1) Copy '$exampleFile' to '$envFile' and set UE_ROOT, or
  2) Set process/user env var UE_ROOT to your Unreal Engine root
     (the folder that contains Engine\Build\BatchFiles\Build.bat).
"@
    }

    if (-not (Test-Path -LiteralPath $ueRoot)) {
        throw "UE_ROOT does not exist: $ueRoot"
    }

    $buildBat = Join-Path $ueRoot 'Engine\Build\BatchFiles\Build.bat'
    if (-not (Test-Path -LiteralPath $buildBat)) {
        throw "Build.bat not found under UE_ROOT. Expected: $buildBat"
    }

    $uproject = Join-Path $repoRoot ($ueProject + '.uproject')
    if (-not (Test-Path -LiteralPath $uproject)) {
        $discovered = Get-ChildItem -LiteralPath $repoRoot -Filter '*.uproject' -File -ErrorAction SilentlyContinue
        if ($discovered.Count -eq 1) {
            $uproject = $discovered[0].FullName
            $ueProject = [System.IO.Path]::GetFileNameWithoutExtension($discovered[0].Name)
            Write-Warning "UE_PROJECT='$ueProject' inferred from $($discovered[0].Name)"
        }
        else {
            Write-Warning "Project file not found yet: $uproject (ok before first scaffold)"
        }
    }

    return [pscustomobject]@{
        RepoRoot   = $repoRoot
        EnvFile    = $envFile
        UE_ROOT    = $ueRoot
        UE_PROJECT = $ueProject
        UE_TARGET  = $ueTarget
        UE_PLATFORM = $uePlatform
        UE_CONFIG  = $ueConfig
        BuildBat   = $buildBat
        UProject   = $uproject
    }
}
