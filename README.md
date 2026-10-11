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

Use `-Side Left` or `-Side Right` to flash only one half. The script builds once with eight parallel jobs, then flashes the same HEX file to each half using `dfu-programmer`, preserving handedness EEPROM. Use `-Jobs 4` to change parallelism or `-BuildOnly` to compile without flashing; the script reports build time. On Windows it disables Make output synchronization, runs recipes through Git Bash, and caches compiler-option probes within each build to avoid LTO mutex errors, shell quoting errors, and repeated compiler checks.

On Windows, Kyria rev1's Atmel DFU bootloader needs the **WinUSB** driver. If QMK waits indefinitely for the device, enter bootloader mode and use Zadig to select `ATm32U4DFU` (USB ID `03EB:2FF4`) and install `WinUSB`. Do not replace the driver for the normal keyboard device. Disconnect the TRRS cable while flashing each half.

## Mac and Windows

The same firmware works on both systems. It automatically detects the USB host and adapts editing shortcuts and Danish symbols. Select **Danish** as the input source on both computers. The Mac angle-bracket mappings send the grave-position key (`KC_GRV`) and Shift with that key, matching the Danish mapping where the Windows ISO key produces `$` and `§`. macOS keyboard-type detection can swap these two positions; if angle brackets still produce `$` and `§` after flashing, use Keyboard Setup Assistant to select **ANSI** for this keyboard.

Automatic detection uses QMK's USB fingerprinting and can guess incorrectly through some hubs or KVM switches. Hold the **NUM** thumb key to override it:

| Mode | Position on the number layer |
| --- | --- |
| Auto | Left hand, top row, innermost key (Canary `b`, QWERTY `t`) |
| Windows | Right hand, top row, innermost key (Canary `z`, QWERTY `y`) |
| Mac | Right hand, middle row, innermost key (Canary `m`, QWERTY `h`) |

Manual overrides persist when unplugged. Select **Auto** again when you want automatic switching. The setting is saved on the USB-connected half; if you move USB to the other half, that half has its own setting. Unknown hosts use Windows mappings. The default build includes Canary only; optional QWERTY selection is independent of the host mode.

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

The symbol layer adapts `@`, `$`, `<`, `>`, `{}`, `[]`, `|`, backslash, and `~` to the Danish Mac layout. Accent keys retain their existing dead-key behavior. Mac desktop switching depends on the corresponding Mission Control shortcuts being enabled.

Flash both halves using the command above to install these changes. After flashing, check copy/paste, `@ $ { } [ ] | \ ~ < >`, and the Danish letter combos on each computer. If detection guesses wrong, select the explicit host mode and retry. Debug builds also log OS detection.

References: [QMK OS detection](https://docs.qmk.fm/features/os_detection), [Apple keyboard shortcuts](https://support.apple.com/en-us/102650), and [Unicode's Danish Mac keyboard data](https://github.com/unicode-org/cldr/blob/release-42/keyboards/osx/da-t-k0-osx.xml).

## Displays and firmware space

The default build enables both 128x64 OLEDs and retains the animated pet, WPM counter, modifier indicators, layer icons/text, OS indicator, symbol guide, matrix scan-rate counter, and 30-second sleep. QWERTY builds also show the layout name. The secondary display shows only the supplied ForeFlight emblem, vertically centered with eight-pixel margins, outside the symbol layer. The symbol guide includes all 30 symbol-layer keys, including the acute accent (`´`) and a solid vertical pipe (`|`). OLED builds define `DEBUG_MATRIX_SCAN_RATE` so the `MS` row shows scans per second; the regular build keeps the USB console disabled. Flash both halves with `./flash.ps1`; inspect the logo, change to/from SYM, type above 40 WPM, and check sleep/wake on the keyboard.

Measured with the local QMK checkout and AVR GCC 15.2.0 for `splitkb/kyria/rev1` on 2026-10-11:

| Build | Flash bytes | Result (28,672-byte application limit) |
| --- | ---: | --- |
| Previous firmware: full autocorrect, OLED off | 27,098 | 1,574 bytes free |
| ForeFlight and all display functions, full autocorrect | 34,784 | 6,112 bytes over; link fails |
| ForeFlight and all display functions, autocorrect omitted | 27,150 | 1,522 bytes free |
| Optimized Canary build, full 425-entry dictionary (before logo text removal/Mac legends) | 32,158 | 3,486 bytes over |
| Optimized Canary build, 175-entry dictionary and all display functions | 28,658 | 14 bytes free |
| Optimized QWERTY/Canary build, 154-entry dictionary and all display functions | 28,660 | 12 bytes free |
| Optimized Canary diagnostic build, autocorrect omitted, all display functions | 27,048 | 1,624 bytes free |

The optimized build removes encoder support (this keyboard has no knobs), QWERTY by default, QMK's unused native OSM/OSL system, unused/duplicate font glyphs, and duplicate animation pixel rows. The compact font is 906 bytes, saving 630 bytes versus the original font while adding Mac modifier legends. Existing glyphs and animation frames preserve their original pixels. Your custom `OS_*` one-shot modifiers, Caps Word/snake case, Mac/Windows detection, word-editing overrides, combos, and media keys remain enabled. RGB, console, command mode, and other unused features are disabled; WPM and deferred execution remain enabled for the animation. LTO is already enabled. See [QMK's AVR size guidance](https://docs.qmk.fm/squeezing_avr).

The OS row shows the effective Mac/Windows mode, including manual overrides and the Windows fallback for unknown hosts. It replaces the layout row when QWERTY is disabled; QWERTY builds show OS on the spare row below the title. The modifier row follows that same mode. Mac mode shows Control (⌃), Option (⌥), outlined Shift (⇧), and Command (⌘), in [Apple's modifier order](https://developer.apple.com/design/human-interface-guidelines/keyboards). Windows mode retains its previous legends and order. Both left/right modifiers and your custom queued `OS_*` modifiers highlight the matching legend.

`QWERTY_ENABLE=no` is the default. Use `./flash.ps1 -Qwerty` or add `-e QWERTY_ENABLE=yes` to `qmk compile` to restore its keymap, NUM top-right base-layout toggle, and display labels. That flag automatically selects a smaller 154-entry dictionary. Both profiles retain all 25 recurring personal rules. Build sizes depend on the QMK revision and compiler; always check the size before flashing, since the default build has very little remaining headroom.

`user/font_source.h` is the editable glyph source; firmware uses the generated compact `user/font.h`. Source slots 0x80–0x83 contain the Mac Command and outlined Shift symbols. After changing font glyphs or adding `OLED_GLYPH_*` references, run `uv run tests/compact_oled_font.py`, then `uv run tests/compact_oled_font.py --check`.

## Autocorrect

Autocorrect is enabled in the default firmware. `user/autocorrection_dict_reduced.txt` includes all 25 recurring personal corrections plus the first 150 original rules, which prioritize common words. Its 175 entries occupy a 2,655-byte trie. The QWERTY profile uses `user/autocorrection_dict_qwerty.txt` with 154 entries (25 personal + 129 original), a 2,296-byte trie. One-off personal candidates stay excluded. To build without autocorrect, pass `-e AUTOCORRECT_ENABLE=no`.

`user/autocorrect_data.h` selects the generated reduced, QWERTY, or full table. The complete 425-entry dictionary remains preserved in `user/autocorrection_dict_extra.txt` and `user/autocorrect_data_full.h`; build it with displays disabled:

```sh
qmk compile -kb splitkb/kyria/rev1 -km frederikrosenberg -e TARGET=k -e KEYBOARD_FILESAFE=k -e OLED_ENABLE=no -e AUTOCORRECT_PROFILE=full
```

The full source preserves all 400 original corrections with their original matching behavior plus 25 recurring personal typos, including `clearence`, `sholud`, and `hcanges`. The reduced profiles select subsets without changing trigger patterns or replacements.

The new personal rules pass checks against Danish, US English, UK English, and broader English word lists, including inflections and fragments around Danish letters. All personal corrections require whole-word matches. Findings for original rules are informational; those rules remain unchanged. Word lists cannot cover every name, identifier, rare word, or new Danish compound. See [the dictionary checks](tests/README.md#autocorrect-dictionary-checks) for regeneration and validation commands.

When compiled with autocorrect enabled, the upper-left `AC_TOGG` key on the NUM layer toggles it; QMK remembers its state. Flash both halves after regenerating/building the dictionary. Test a preserved correction such as `beacuse`, a new correction such as `sholud` followed by Space, and unchanged words such as `borde`, `broder`, `maraton`.

## Debugging missed keypresses

Debugging is optional and is controlled by the flash script; the regular firmware does not include the console logging. Debug builds omit autocorrect, and the script prints a notice. `AC_TOGG` cannot enable autocorrect in these builds. Reflash without `-Debug` to remove diagnostics and restore the reduced dictionary. This also applies to direct builds using `KEYBOARD_DEBUG=1`. Flash both halves with diagnostics enabled:

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
