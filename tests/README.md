# Host OS tests

These host-side tests exercise automatic detection callbacks, persistent overrides, Danish Mac symbols, shortcuts, releases across mode changes, and queued one-shot modifiers. Word-editing tests also compile QMK's real key-override engine and check the resulting key/modifier reports, including held Control, Shift selection, terminal shortcuts, and release ordering. The small API shim models host reports and timers.

App-switching tests check NAV held with repeated NUM taps, backward switching with queued/held Shift, release ordering, cancellation and mode changes, individual thumb-layer keys, and unchanged Copy behavior.

From this repository in PowerShell, with Clang and the Visual Studio C++ tools installed:

```powershell
$qmkTestRoot = Join-Path $HOME 'qmk_firmware'
& 'C:\Program Files\LLVM\bin\clang.exe' -std=c11 -Wall -Wextra -Werror `
    '-DQMK_KEYBOARD_H="qmk_test.h"' -Itests -Iuser `
    "-I$qmkTestRoot/quantum/process_keycode" `
    "-I$qmkTestRoot/quantum" "-I$qmkTestRoot/quantum/keymap_extras" `
    "-I$qmkTestRoot/quantum/sequencer" "-I$qmkTestRoot/tmk_core/protocol" `
    tests/host_os_test.c user/features/host_os.c user/features/oneshot.c `
    "$qmkTestRoot/quantum/process_keycode/process_key_override.c" `
    -o "$env:TEMP/qmk-host-os-test.exe"
if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed' }
& "$env:TEMP/qmk-host-os-test.exe"
if ($LASTEXITCODE -ne 0) { throw 'Host OS tests failed' }
```

On macOS/Linux, the same sources and include paths work with `cc` or `clang`; keep the quoted header definition and select a local executable output path.

Also compile the Kyria firmware as described in the main README. USB fingerprint accuracy and host input-source settings require checking on the physical keyboard with both computers.

## Autocorrect dictionary checks

The active source is `user/autocorrection_dict_extra.txt`, compiled into `user/autocorrect_data.h`. It includes all 400 original corrections with their original matching behavior plus the 25 recurring personal corrections. The other personal candidates remain inactive. Check both sources with:

```powershell
uv run --with spylls==0.1.7 --with english-words==2.0.2 --with pyahocorasick==2.3.1 `
    tests/validate_autocorrect.py `
    --cache "$env:TEMP/qmk-autocorrect-validation" `
    --report tests/autocorrect_validation.json
if ($LASTEXITCODE -ne 0) { throw 'Autocorrect safety checks failed' }
```

The checker downloads a pinned revision of LibreOffice's Danish, US English, and UK English Hunspell dictionaries and verifies their SHA-256 hashes. For personal candidates, it rejects valid dictionary words, inflections and recognized compounds, extra repeated-letter corrections, double-letter-count-only corrections, missing-i-only corrections, and conflicting rules. All existing generic corrections are intentionally preserved at the user's request; findings for those rules are informational and do not fail the check. The active trigger patterns are checked against English word lists and overgenerated Danish/English affix forms, including ASCII fragments around non-ASCII letters: QMK can treat Danish letters as boundaries, so whole-word markers alone are insufficient protection.

All personal entries require both word boundaries. No original rules are removed or narrowed. Dictionary checking cannot cover every name, identifier, rare word or newly formed Danish compound. Test Danish and English typing on the physical keyboard after flashing.

Regenerate and check the serialized dictionary after editing the active source:

```powershell
uvx --with english-words==2.0.2 qmk generate-autocorrect-data `
    user/autocorrection_dict_extra.txt -o user/autocorrect_data.h
if ($LASTEXITCODE -ne 0) { throw 'Autocorrect generation failed' }
uv run tests/check_autocorrect_data.py
if ($LASTEXITCODE -ne 0) { throw 'Generated trie checks failed' }
```

The generated-trie checks exercise all 25 personal replacements, preserved common generic corrections, and selected English/Danish words protected by the new entries. They also verify every encoded rule directly, since some original partial rules intentionally shadow longer rules during typing. The active dictionary has 425 entries. The bilingual audit reports potential matches in the original rules for information; only personal-candidate problems fail validation.
