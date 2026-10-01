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
