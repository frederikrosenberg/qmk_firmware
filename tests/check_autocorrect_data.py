"""Check the generated QMK trie, personal corrections, and protected words."""

import argparse
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--profile", choices=["full", "reduced", "qwerty"], default="full")
args = parser.parse_args()
header = (ROOT / f"user/autocorrect_data_{args.profile}.h").read_text(encoding="utf-8")
dictionary = "extra" if args.profile == "full" else args.profile
array = re.search(r"autocorrect_data\[DICTIONARY_SIZE\].*?=\s*\{(.*?)\};", header, re.S)[1]
DATA = [int(value.strip(), 0) for value in array.split(",") if value.strip()]
MAX_LENGTH = int(re.search(r"#define AUTOCORRECT_MAX_LENGTH (\d+)", header)[1])
assert len(DATA) == int(re.search(r"#define DICTIONARY_SIZE (\d+)", header)[1])


def match(buffer):
    """Walk QMK's serialized reverse trie at each key press."""
    state = 0
    code = DATA[state]
    for key in reversed(buffer[-MAX_LENGTH:]):
        if code & 64:
            code &= 63
            while code != key:
                state += 3
                code = DATA[state]
                if not code:
                    return None
            state = DATA[state + 1] | DATA[state + 2] << 8
            code = DATA[state]
        elif code != key:
            return None
        else:
            state += 1
            if DATA[state] == 0:
                state += 1
            code = DATA[state]
        if code & 128:
            end = DATA.index(0, state + 1)
            return code & 63, bytes(DATA[state + 1:end]).decode("ascii")
    return None


def stream(word):
    buffer = []
    # Conservative projection includes Danish letters as possible boundaries.
    for char in ":" + re.sub("[^a-z]", ":", word.lower()) + ":":
        buffer.append(0x2C if char == ":" else ord(char) - ord("a") + 4)
        found = match(buffer)
        if found is not None:
            return found
    return None


personal = (ROOT / "user/autocorrection_dict_personal.txt").read_text(encoding="utf-8")
recurring = personal.split("# One-off", 1)[0]
count = 0
for line in recurring.splitlines():
    if "->" not in line or line.lstrip().startswith("#"):
        continue
    trigger, correction = (part.strip() for part in line.split("->"))
    typo = trigger.strip(":")
    backspaces, replacement = stream(typo)
    assert (typo[:-backspaces] if backspaces else typo) + replacement == correction, trigger
    count += 1
assert count > 0

for word in [
    "borde", "broder", "broders", "brodersøn", "caféborde", "maraton", "tese",
]:
    assert stream(word) is None, word

# Common old corrections must still work, and safe partial rules are preserved.
for typo in ("beacuse", "becuase", "woudl", "peopel", "whcih"):
    assert stream(typo) is not None, typo
if args.profile == "full":
    assert stream("recieve") is not None

# Verify each encoded rule directly. Some longstanding partial rules shadow
# longer legacy rules during typing; preserving that behavior is intentional.
active_count = 0
for line in (ROOT / f"user/autocorrection_dict_{dictionary}.txt").read_text(encoding="utf-8").splitlines():
    if "->" not in line or line.lstrip().startswith("#"):
        continue
    trigger, correction = (part.strip() for part in line.split("->"))
    typo = trigger.strip(":")
    buffer = [0x2C if char == ":" else ord(char) - ord("a") + 4
              for char in ":" + trigger.lstrip(":")]
    found = match(buffer)
    assert found is not None, trigger
    backspaces, replacement = found
    host_text = typo if trigger.endswith(":") else typo[:-1]
    assert (host_text[:-backspaces] if backspaces else host_text) + replacement == correction, trigger
    active_count += 1
assert active_count == int(re.search(r"Autocorrection dictionary \((\d+) entries\)", header)[1])

print(f"Generated trie: all {active_count} active entries, {count} personal entries, and protected words passed")
