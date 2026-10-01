# Keyboard layout

This is the keyboard layout I use for my split keyboards. The structure is designed to support multiple keyboards while having a shared configuration.

## Setup

This repository is a userspace overlay for a separate QMK checkout. Clone the latest QMK firmware to `~/qmk_firmware`, then run `setup.sh` from this repository. The script links the userspace and Kyria keymap into the current revisioned QMK layout:

```sh
./setup.sh
```

The default target is Kyria `rev1`, which matches the original pre-revision layout used by this repository. Set `KYRIA_REVISION=rev2` or `KYRIA_REVISION=rev3` when using another Kyria revision.

Compile with:

```sh
qmk compile -kb splitkb/kyria/rev1 -km frederikrosenberg
```

On Windows Git Bash, use a short build target if the default command exceeds the shell's command-line length limit:

```sh
qmk compile -kb splitkb/kyria/rev1 -km frederikrosenberg -e TARGET=k -e KEYBOARD_FILESAFE=k
```

To flash both halves after the handedness EEPROM values have been set:

```powershell
powershell -ExecutionPolicy Bypass -File .\flash.ps1
```

Use `-Side Left` or `-Side Right` to flash only one half. The script uses the ordinary `dfu` target, so it does not change handedness.

On Windows, Kyria rev1's Atmel DFU bootloader needs the **WinUSB** driver. If QMK waits indefinitely for the device, enter bootloader mode and use Zadig to select `ATm32U4DFU` (USB ID `03EB:2FF4`) and install `WinUSB`. Do not replace the driver for the normal keyboard device. Disconnect the TRRS cable while flashing each half.

## Mac and Windows

The same firmware works on both systems. It automatically detects the USB host and adapts editing shortcuts and Danish symbols. Select **Danish** as the input source on both computers. On macOS, identify the external keyboard as **ISO (European)** if Keyboard Setup Assistant asks; choosing ANSI can swap `<` and `$`.

Automatic detection uses QMK's USB fingerprinting and can guess incorrectly through some hubs or KVM switches. Hold the **NUM** thumb key to override it:

| Mode | Position on the number layer |
| --- | --- |
| Auto | Left hand, top row, innermost key (Canary `b`, QWERTY `t`) |
| Windows | Right hand, top row, innermost key (Canary `z`, QWERTY `y`) |
| Mac | Right hand, middle row, innermost key (Canary `m`, QWERTY `h`) |

Manual overrides persist when unplugged. Select **Auto** again when you want automatic switching. The setting is saved on the USB-connected half; if you move USB to the other half, that half has its own setting. Unknown hosts use Windows mappings. Canary/QWERTY selection is independent of the host mode.

| Existing key | Windows | Mac |
| --- | --- | --- |
| Undo / Cut / Copy / Paste | Ctrl+Z / X / C / V | Command+Z / X / C / V |
| `OS_CTR` | Control | Control |
| `OS_MOD` | Windows key | Command |
| `OS_ALT` | Alt | Option |
| Home / End | Home / End | Command+Left / Right (start/end of line) |
| Desktop left / right | Ctrl+Windows+Left / Right | Control+Left / Right (Mission Control spaces) |
| Ctrl+Alt+Delete position | Ctrl+Alt+Delete | Command+Option+Escape (Force Quit) |
| Ctrl+Backspace | Delete previous word | Option+Backspace (delete previous word) |
| Ctrl+Left / Right | Move by word | Option+Left / Right (move by word) |
| Ctrl+Shift+Left / Right | Select by word | Option+Shift+Left / Right (select by word) |
| Hold NAV, tap NUM | Alt+Tab (switch apps) | Command+Tab (switch apps) |

For app switching, **hold NAV and tap NUM** repeatedly to cycle through apps. Release NAV to select the app. Add Shift to cycle backward; a queued `OS_SFT` applies to one NUM tap. NAV and NUM still work as individual layer keys. The NAV-then-NUM chord is reserved for app switching, and ordinary Alt/Option combinations retain their existing behavior.

Word editing works with the queued or held `OS_CTR` modifier and either physical Control key. In Mac mode, Control is suppressed only for these combinations; held keys repeat normally. Other Control combinations, including Ctrl+C in a terminal, keep their usual behavior. Chords that also include Alt/Option or Windows/Command are passed through unchanged.

The symbol layer adapts `@`, `$`, `{}`, `[]`, `|`, backslash, and `~` to the Danish Mac layout. Accent keys retain their existing dead-key behavior. Mac desktop switching depends on the corresponding Mission Control shortcuts being enabled.

Flash both halves using the command above to install these changes. After flashing, check copy/paste, `@ $ { } [ ] | \ ~ < >`, and the Danish letter combos on each computer. If detection guesses wrong, select the explicit host mode and retry. Debug builds also log OS detection. OLED remains disabled to keep the rev1 firmware within its flash capacity.

References: [QMK OS detection](https://docs.qmk.fm/features/os_detection), [Apple keyboard shortcuts](https://support.apple.com/en-us/102650), and [Unicode's Danish Mac keyboard data](https://github.com/unicode-org/cldr/blob/release-42/keyboards/osx/da-t-k0-osx.xml).

## Debugging missed keypresses

Debugging is optional and is controlled by the flash script; the regular firmware does not include the console logging. Flash both halves with diagnostics enabled:

```powershell
.\flash.ps1 -Debug
```

After flashing, connect the keyboard and run this from the QMK checkout in a second terminal:

```sh
uvx qmk console
```

Press a key that sometimes fails and look for both the matrix dump and a `KEY row=... col=... kc=... down` line. If the matrix dump does not change, the switch, wiring, or matrix scan is the likely problem. If the matrix changes but no `KEY` event appears, the issue is later in QMK's event processing. If the `KEY` event appears but the computer receives nothing, investigate the keymap logic or host-side behavior. Disconnect the TRRS cable while inspecting each half separately if the problem only affects one side.

---

Shout-out to metheon (https://github.com/metheon/qmk_layout) for the inspiration for the code layout.
