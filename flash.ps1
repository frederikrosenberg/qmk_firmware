param(
    [ValidateSet('Left', 'Right', 'Both')]
    [string]$Side = 'Both',

    [string]$QmkFirmwareDir = (Join-Path $HOME 'qmk_firmware'),

    [string]$QmkToolsDir = (Join-Path $HOME '.qmk\bin'),

    [switch]$Debug,

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

if (-not $DryRun -and -not (Get-Command dfu-programmer -ErrorAction SilentlyContinue)) {
    throw "dfu-programmer was not found. Install QMK's Windows flashing utilities, or use QMK Toolbox with k.hex."
}

Write-Host 'Windows DFU driver: the bootloader must use WinUSB (Zadig device ATm32U4DFU, USB ID 03EB:2FF4).'
Write-Host 'Connect only the half being flashed; disconnect the TRRS cable between halves.'

$keyboard = 'splitkb/kyria/rev1'
$keymap = 'frederikrosenberg'
$arguments = @(
    'qmk', 'flash',
    '-kb', $keyboard,
    '-km', $keymap,
    '-bl', 'dfu',
    '-e', 'TARGET=k',
    '-e', 'KEYBOARD_FILESAFE=k'
)

if ($Debug) {
    $arguments += @('-e', 'KEYBOARD_DEBUG=1')
}

Push-Location -LiteralPath $QmkFirmwareDir
try {
    $sides = @(
        if ($Side -eq 'Both') { 'Left'; 'Right' } else { $Side }
    )

    for ($index = 0; $index -lt $sides.Count; $index++) {
        $currentSide = $sides[$index]
        $buildMode = if ($Debug) { 'debug' } else { 'normal' }
        Write-Host "Flashing $currentSide half ($($index + 1) of $($sides.Count)) with $buildMode firmware. Connect it by USB and press reset when QMK waits for the bootloader. No Enter key is needed between halves."

        if ($DryRun) {
            Write-Host "DRY RUN: uvx $($arguments -join ' ')"
            continue
        }

        & uvx @arguments
        if ($LASTEXITCODE -ne 0) {
            throw "QMK flashing failed for the $currentSide half with exit code $LASTEXITCODE."
        }
    }
}
finally {
    Pop-Location
}
