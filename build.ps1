#Requires -Version 5.1
<#
    build.ps1 - one-click build & test with MSVC (cl.exe).

    Usage:
        .\build.ps1                 # build everything, then run all tests
        .\build.ps1 -Target test    # build + run tests only
        .\build.ps1 -Target bench   # build + run the benchmark
        .\build.ps1 -Target bench -N 100000

    If you prefer gcc/MinGW instead of MSVC, use the Makefile:
        make test
#>
[CmdletBinding()]
param(
    [ValidateSet('all', 'test', 'bench')]
    [string]$Target = 'all',

    [int]$N = 20000
)

$ErrorActionPreference = 'Stop'

$Here = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location -LiteralPath $Here

$Build = Join-Path $Here 'build'

function Find-VcVars {
    $found = @()

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $installs = & $vswhere -latest -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath 2>$null
        foreach ($i in $installs) {
            if ($i) { $found += (Join-Path $i.Trim() 'VC\Auxiliary\Build\vcvars64.bat') }
        }
    }

    $found += Get-ChildItem 'C:\Program Files\Microsoft Visual Studio\*\*\VC\Auxiliary\Build\vcvars64.bat' `
        -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName }
    $found += Get-ChildItem 'C:\Program Files (x86)\Microsoft Visual Studio\*\*\VC\Auxiliary\Build\vcvars64.bat' `
        -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName }

    foreach ($c in $found) {
        if ($c -and (Test-Path -LiteralPath $c)) { return $c }
    }
    return $null
}

function Quote([string]$s) { return '"' + $s + '"' }

$vcvars = Find-VcVars
if (-not $vcvars) {
    Write-Host '[error] vcvars64.bat not found.' -ForegroundColor Red
    Write-Host '        Install Visual Studio with "Desktop development with C++",'
    Write-Host '        or use gcc/MinGW:  make test'
    exit 1
}

if (-not (Test-Path -LiteralPath $Build)) {
    New-Item -ItemType Directory -Path $Build -Force | Out-Null
}

$Flags = '/nologo /W3 /O2 /std:c11 /utf-8 /D_CRT_SECURE_NO_WARNINGS /I' + (Quote (Join-Path $Here 'include'))

$Sources = @(
    (Join-Path $Here 'src\task.c'),
    (Join-Path $Here 'src\seqlist.c'),
    (Join-Path $Here 'src\linkedlist.c')
) | ForEach-Object { Quote $_ }
$SrcArgs = $Sources -join ' '

$TestList  = Quote (Join-Path $Here 'tests\test_list.c')
$TestCross = Quote (Join-Path $Here 'tests\test_cross.c')
$BenchSrc  = Quote (Join-Path $Here 'bench\bench.c')
# 作业本：只拿 my-impl/linkedlist.c + src/task.c，不掺 src/linkedlist.c
$MySrc     = (Quote (Join-Path $Here 'src\task.c')) + ' ' + (Quote (Join-Path $Here 'my-impl\linkedlist.c'))

# Build a temporary .bat so that "call vcvars64.bat" and cl.exe share one
# cmd session (environment variables set by vcvars do not propagate to PS).
$lines = New-Object System.Collections.Generic.List[string]
$lines.Add('@echo off')
$lines.Add('call "' + $vcvars + '" >nul')
$lines.Add('cd /d "' + $Build + '"')

if ($Target -in @('all', 'test')) {
    $lines.Add("cl $Flags /DUSE_SEQLIST   /Fe:test-seq.exe    $SrcArgs $TestList")
    $lines.Add("cl $Flags /DUSE_LINKEDLIST /Fe:test-linked.exe $SrcArgs $TestList")
    $lines.Add("cl $Flags /Fe:test-cross.exe $SrcArgs $TestCross")
    $lines.Add("cl $Flags /DUSE_LINKEDLIST /Fe:test-my.exe $MySrc $TestList")
}
if ($Target -in @('all', 'bench')) {
    $lines.Add("cl $Flags /Fe:bench.exe $SrcArgs $BenchSrc")
}

$batPath = Join-Path $Build '_build.bat'
Set-Content -LiteralPath $batPath -Value $lines -Encoding Ascii

Write-Host ''
Write-Host "== building (target: $Target) ==" -ForegroundColor Cyan
& cmd.exe /c "`"$batPath`""
if ($LASTEXITCODE -ne 0) {
    Write-Host "[error] compilation failed (exit $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}

# Chinese literals in the test output are UTF-8; make the console agree.
try { [Console]::OutputEncoding = [System.Text.Encoding]::UTF8 } catch { }

if ($Target -in @('all', 'test')) {
    Push-Location -LiteralPath $Build
    $failed = 0
    foreach ($exe in @('test-seq.exe', 'test-linked.exe', 'test-cross.exe', 'test-my.exe')) {
        if (-not (Test-Path -LiteralPath $exe)) { continue }
        & (Join-Path $Build $exe)
        if ($LASTEXITCODE -ne 0) { $failed++ }
    }
    Pop-Location

    Write-Host ''
    if ($failed -gt 0) {
        Write-Host "[failed] $failed test binary/binaries reported errors" -ForegroundColor Red
        exit 1
    }
    Write-Host '[ok] every suite passed' -ForegroundColor Green
}

if ($Target -eq 'bench') {
    & (Join-Path $Build 'bench.exe') $N
}

exit 0
