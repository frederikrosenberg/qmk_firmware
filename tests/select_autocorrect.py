"""Generate the largest common-word prefix within a measured trie byte budget.

Always include the 25 recurring personal rules. The original dictionary lists
common-word corrections first; one-off personal candidates stay excluded.
This budgets the table only. The firmware must still be compiled and size-checked.
"""
import argparse
import re
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--max-bytes", type=int, required=True)
parser.add_argument("--profile", choices=["reduced", "qwerty"], default="reduced")
args = parser.parse_args()
source = (ROOT / "user/autocorrection_dict_extra.txt").read_text()
generic_text, personal_text = source.split("# Recurring personal corrections", 1)
rules = lambda text: [line.strip() for line in text.splitlines()
                      if "->" in line and not line.lstrip().startswith("#")]
generic, personal = rules(generic_text), rules(personal_text)
assert len(generic) == 400 and len(personal) == 25
license_text = source.split("# All 400 original rules", 1)[0]
with tempfile.TemporaryDirectory(prefix="qmk-reduced-autocorrect-") as directory:
    temp = Path(directory)

    def generate(count):
        text = license_text + "# Reduced OLED dictionary: all recurring personal rules, then common words.\n"
        text += f"# {len(personal)} personal + {count} original rules; regenerate with tests/select_autocorrect.py.\n\n"
        text += "\n".join(personal + generic[:count]) + "\n"
        (temp / "dictionary.txt").write_text(text, encoding="utf-8")
        result = subprocess.run(
            ["uvx", "--with", "english-words==2.0.2", "qmk", "generate-autocorrect-data",
             str(temp / "dictionary.txt"), "-o", str(temp / "data.h")],
            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if result.returncode:
            raise RuntimeError(result.stdout)
        header = (temp / "data.h").read_text()
        size = int(re.search(r"#define DICTIONARY_SIZE (\d+)", header)[1])
        return size, text, header

    low, high = 0, len(generic)
    assert generate(0)[0] <= args.max_bytes, "Personal rules alone exceed the table budget"
    while low < high:
        middle = (low + high + 1) // 2
        if generate(middle)[0] <= args.max_bytes:
            low = middle
        else:
            high = middle - 1
    size, text, header = generate(low)
    (ROOT / f"user/autocorrection_dict_{args.profile}.txt").write_text(text, encoding="utf-8")
    (ROOT / f"user/autocorrect_data_{args.profile}.h").write_text(header, encoding="utf-8")
    print(f"Reduced dictionary: {len(personal) + low} entries ({len(personal)} personal + "
          f"{low} original), {size}/{args.max_bytes} table bytes")
