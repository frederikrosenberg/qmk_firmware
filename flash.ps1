param(
    [ValidateSet('Left', 'Right', 'Both')]
    [string]$Side = 'Both',

    [string]$QmkFirmwareDir = (Join-Path $HOME 'qmk_firmware'),

    [string]$QmkToolsDir = (Join-Path $HOME '.qmk\bin'),

    [ValidateRange(1, 256)]
    [int]$Jobs = 8,

    [switch]$Debug,

    [switch]$Qwerty,

    [switch]$BuildOnly,

    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $QmkFirmwareDir)) {
    throw "QMK checkout not found: $QmkFirmwareDir"
}

$gitUnixBin = 'C:\Program Files\Git\usr\bin'
$gitBin = 'C:\Program Files\Git\bin'
$pathEntries = @($QmkToolsDir, $gitUnixBin, $gitBin) + $env:Path.Split(';')
$env:Path = (($pathEntries | Where-Object { $_ -and (Test-Path -LiteralPath $_) } | Select-Object -Unique) -join ';')

if (-not (Get-Command uvx -ErrorAction SilentlyContinue)) {
    throw 'uvx was not found. Install uv or add it to PATH before running this script.'
}

if (-not $DryRun -and -not $BuildOnly -and -not (Get-Command dfu-programmer -ErrorAction SilentlyContinue)) {
    throw "dfu-programmer was not found. Install QMK's Windows flashing utilities, or use QMK Toolbox with k.hex."
}

$bashPath = Join-Path $gitBin 'bash.exe'
if (-not (Test-Path -LiteralPath $bashPath)) {
    throw "Git Bash was not found: $bashPath"
}

if (-not $BuildOnly) {
    Write-Host 'Windows DFU driver: the bootloader must use WinUSB (Zadig device ATm32U4DFU, USB ID 03EB:2FF4).'
    Write-Host 'Connect only the half being flashed; disconnect the TRRS cable between halves.'
}

$keyboard = 'splitkb/kyria/rev1'
$keymap = 'frederikrosenberg'
$arguments = @(
    'qmk', 'compile',
    # QMK adds --output-sync=target with -j > 1. Native Windows Make's
    # sync mutex fails in AVR GCC's LTO sub-make; use MAKEFLAGS instead.
    '-j', '1',
    '-kb', $keyboard,
    '-km', $keymap,
    '-e', 'TARGET=k',
    '-e', 'KEYBOARD_FILESAFE=k',
    # Stable compiler flags preserve incremental builds across terminals.
    '-e', 'COLOR=false',
    # Without output synchronization, per-file progress interleaves. Keep
    # compiler diagnostics and the size check; report timing ourselves.
    '-e', 'SILENT=true',
    '-e', 'cc-option=$(call qmk-cached-cc-option,$(1),$(2),$(3))',
    '-e', "SHELL=$($bashPath.Replace('\', '/'))",
    # Force Make to use Bash even for simple commands. Native execution
    # misinterprets QMK's quoting/escapes (including expr's escaped '*').
    '-e', '.SHELLFLAGS=--noprofile --norc -c'
)

if ($Debug) {
    $arguments += @('-e', 'KEYBOARD_DEBUG=1')
    Write-Host 'DEBUG BUILD: Console diagnostics and OLED functions are included. Autocorrect is omitted; AC_TOGG cannot enable it.' -ForegroundColor Yellow
}
else {
    Write-Host 'NORMAL BUILD: All OLED functions and a reduced autocorrect dictionary are included; AC_TOGG toggles autocorrect.'
}

if ($Qwerty) {
    $arguments += @('-e', 'QWERTY_ENABLE=yes')
    Write-Host 'QWERTY support is included with a smaller autocorrect dictionary. NUM top-right toggles the base layout.'
}
else {
    Write-Host 'Canary only: QWERTY support is omitted to make room for more autocorrect entries.'
}

$previousMakeFlags = $env:MAKEFLAGS
$previousMakeFiles = $env:MAKEFILES
$probeCacheFile = ".build/flash-cc-cache-$PID.mk"
Push-Location -LiteralPath $QmkFirmwareDir
try {
    $env:MAKEFLAGS = "--jobs=$Jobs --output-sync=none"
    # QMK's cc-option probes run repeatedly while expanding each object's
    # flags. Cache results within this build, keeping QMK's actual probe.
    $env:MAKEFILES = $probeCacheFile
    if (-not $DryRun) {
        $supportFile = Get-Content -LiteralPath 'builddefs\support.mk' -Raw
        if ($supportFile -notmatch '(?m)^cc-option\s*=') {
            throw 'Cannot locate QMK compiler-option probes in builddefs/support.mk.'
        }
        $probeCache = ($supportFile -replace '(?m)^cc-option\s*=', 'qmk-original-cc-option =') + @'

qmk-probe-key = qmk-probe-$(subst =,_,$(1)|$(2)|$(3))
qmk-cached-cc-option = $(if $(filter undefined,$(origin $(call qmk-probe-key,$(1),$(2),$(3)))),$(eval $(call qmk-probe-key,$(1),$(2),$(3)) := $(call qmk-original-cc-option,$(1),$(2),$(3))))$($(call qmk-probe-key,$(1),$(2),$(3)))
'@
        New-Item -ItemType Directory -Path '.build' -Force | Out-Null
        [System.IO.File]::WriteAllText((Join-Path (Get-Location).Path $probeCacheFile), $probeCache)
    }
    Write-Host "Building firmware once with $Jobs parallel jobs."
    $buildTimer = [System.Diagnostics.Stopwatch]::StartNew()
    if ($DryRun) {
        Write-Host "DRY RUN: MAKEFLAGS=$env:MAKEFLAGS MAKEFILES=$env:MAKEFILES uvx $($arguments -join ' ')"
    }
    else {
        & uvx @arguments
        if ($LASTEXITCODE -ne 0) {
            throw "QMK compilation failed with exit code $LASTEXITCODE."
        }
        if (-not (Test-Path -LiteralPath '.build\k.hex')) {
            throw 'QMK did not produce .build\k.hex.'
        }
        Write-Host ('Build completed in {0:N1} seconds.' -f $buildTimer.Elapsed.TotalSeconds)
    }

    if ($BuildOnly) { return }

    $sides = @(
        if ($Side -eq 'Both') { 'Left'; 'Right' } else { $Side }
    )

    for ($index = 0; $index -lt $sides.Count; $index++) {
        $currentSide = $sides[$index]
        $buildMode = if ($Debug) { 'debug' } else { 'normal' }
        Write-Host "Flashing $currentSide half ($($index + 1) of $($sides.Count)) with $buildMode firmware. Connect it by USB and press reset. Waiting for the DFU bootloader; Ctrl+C cancels."

        if ($DryRun) {
            Write-Host 'DRY RUN: wait for dfu-programmer atmega32u4 get bootloader-version'
            Write-Host 'DRY RUN: dfu-programmer atmega32u4 erase --force'
            Write-Host 'DRY RUN: dfu-programmer atmega32u4 flash .build\k.hex'
            Write-Host 'DRY RUN: dfu-programmer atmega32u4 reset'
            continue
        }

        do {
            # No device is an expected polling result. Windows PowerShell
            # can otherwise turn redirected native stderr into an exception.
            $pollErrorPreference = $ErrorActionPreference
            try {
                $ErrorActionPreference = 'SilentlyContinue'
                & dfu-programmer atmega32u4 get bootloader-version *> $null
                $bootloaderFound = $LASTEXITCODE -eq 0
            }
            finally {
                $ErrorActionPreference = $pollErrorPreference
            }
            if (-not $bootloaderFound) { Start-Sleep -Milliseconds 500 }
        } until ($bootloaderFound)

        foreach ($dfuArguments in @(
            @('atmega32u4', 'erase', '--force'),
            @('atmega32u4', 'flash', '.build\k.hex'),
            @('atmega32u4', 'reset')
        )) {
            & dfu-programmer @dfuArguments
            if ($LASTEXITCODE -ne 0) {
                throw "DFU $($dfuArguments[1]) failed for the $currentSide half with exit code $LASTEXITCODE."
            }
        }
        # Let the just-flashed half leave bootloader mode before polling again.
        Start-Sleep -Seconds 1
    }
}
finally {
    $env:MAKEFLAGS = $previousMakeFlags
    $env:MAKEFILES = $previousMakeFiles
    if (-not $DryRun -and (Test-Path -LiteralPath $probeCacheFile)) {
        Remove-Item -LiteralPath $probeCacheFile
    }
    Pop-Location
}
