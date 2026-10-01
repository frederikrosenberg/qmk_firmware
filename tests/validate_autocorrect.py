"""Conservative bilingual audit of the active and personal QMK dictionaries.

Run with uv run --with spylls==0.1.7 --with english-words==2.0.2 --with pyahocorasick==2.3.1
tests/validate_autocorrect.py --cache <directory> [--report <json path>].
Only the pinned public dictionaries are downloaded; no user text is sent.
"""

import argparse
import difflib
import hashlib
from importlib.metadata import version
import json
import re
import ahocorasick
from pathlib import Path
from urllib.request import urlopen

from english_words import get_english_words_set
from spylls.hunspell import Dictionary

ROOT = Path(__file__).resolve().parents[1]
REVISION = "32b006a2c22a4ac7e8ed3f03346f7b3d85a970a4"
FILES = {
    "da_DK.aff": "ee4fe193d19b72aa64bc0d2b9bcdf44ee44f11d9b4e7dcf6da36fd0e91528561",
    "da_DK.dic": "298aabeca362ee2c71836c5c7748d22080a460bd7193e6fbd2471f27a70d0fd3",
    "en_US.aff": "e746c882dd6f303c2c46e7452804b9201115a6942cfeb15f18f8edf774d2e24e",
    "en_US.dic": "f0b1a234bd178bdd01875b2a392a9647f888b8fe879f79c52aae62c2759b3647",
    "en_GB.aff": "0fd6ed120ef28957847d98ba5149b117e27116cf81b5aa36208453f6755a36fd",
    "en_GB.dic": "04e90f34f5263bf26780e9c4a442e9ad16584e227af49ddd1b3b21b01df5b29c",
}
PROTECTED = {"agin": "English dialect spelling of again"}


def entries(path):
    result = []
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if line.strip() and not line.lstrip().startswith("#"):
            trigger, replacement = (part.strip() for part in line.split("->", 1))
            result.append((trigger, replacement, number))
    return result


def repeated_letter_change(typo, correction):
    """Reject extra repetitions even alongside another error, and double-count-only changes."""
    def runs(word):
        result = {}
        for match in re.finditer(r"(.)\1*", word):
            result[match[1]] = max(result.get(match[1], 0), len(match[0]))
        return result

    original, expected = runs(typo), runs(correction)
    if any(length > max(1, expected.get(char, 0)) for char, length in original.items()):
        return True
    if re.sub(r"(.)\1+", r"\1", typo) == re.sub(r"(.)\1+", r"\1", correction):
        return True
    changes = [op for op in difflib.SequenceMatcher(None, typo, correction).get_opcodes() if op[0] != "equal"]
    return bool(changes) and all(
        op == "delete" and all(char in correction for char in typo[start:end])
        for op, start, end, _, _ in changes
    )


def load_dictionaries(cache):
    cache.mkdir(parents=True, exist_ok=True)
    sources = {}
    for name, digest in FILES.items():
        folder = "da_DK" if name.startswith("da_DK") else "en"
        url = f"https://raw.githubusercontent.com/LibreOffice/dictionaries/{REVISION}/{folder}/{name}"
        path = cache / name
        if not path.exists():
            with urlopen(url, timeout=60) as response:
                path.write_bytes(response.read())
        if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
            raise ValueError(f"Dictionary hash mismatch: {path}")
        sources[name] = {"url": url, "sha256": digest}
    dictionaries = {language: Dictionary.from_files(str(cache / language)) for language in ("da_DK", "en_US", "en_GB")}
    return dictionaries, sources


def danish_forms(dictionary):
    """Overgenerate roots and up to two suffixes plus one prefix, conservatively.

    Includes continuation classes and prefix/suffix combinations. Forms that
    require compound context or are forbidden are deliberately not excluded:
    overgeneration is safer for rejecting questionable autocorrect entries.
    Arbitrarily long productive compounds are not enumerated.
    """
    aff = dictionary.aff
    for word in dictionary.dic.words:
        stack = [(word.stem, word.flags, 0, 0)]
        visited = set()
        while stack:
            text, flags, prefixes, suffixes = stack.pop()
            state = (text, frozenset(flags), prefixes, suffixes)
            if state in visited:
                continue
            visited.add(state)
            yield text
            for flag in flags:
                if suffixes < 2:
                    for rule in aff.SFX.get(flag, []):
                        if rule.cond_regexp.search(text) and (not rule.strip or text.endswith(rule.strip)):
                            base = text[:-len(rule.strip)] if rule.strip else text
                            next_flags = set(rule.flags)
                            if rule.crossproduct:
                                next_flags |= {item for item in flags if item in aff.PFX}
                            stack.append((base + rule.add, next_flags, prefixes, suffixes + 1))
                if prefixes < 1:
                    for rule in aff.PFX.get(flag, []):
                        if rule.cond_regexp.search(text) and (not rule.strip or text.startswith(rule.strip)):
                            base = text[len(rule.strip):] if rule.strip else text
                            next_flags = set(rule.flags)
                            if rule.crossproduct:
                                next_flags |= {item for item in flags if item in aff.SFX}
                            stack.append((rule.add + base, next_flags, prefixes + 1, suffixes))


def audit(cache):
    personal = entries(ROOT / "user/autocorrection_dict_personal.txt")
    existing = entries(ROOT / "user/autocorrection_dict_extra.txt")
    dictionaries, sources = load_dictionaries(cache)
    english = get_english_words_set(["web2", "gcide"], lower=True, alpha=True)
    problems = {}

    def problem(trigger, reason):
        reasons = problems.setdefault(trigger, [])
        if reason not in reasons:
            reasons.append(reason)

    bare = {trigger.strip(":"): trigger for trigger, _, _ in personal}
    targets = {trigger for trigger, _, _ in personal}
    if len(targets) != len(personal):
        raise ValueError("Duplicate personal trigger")
    for trigger, correction, _ in personal:
        typo = trigger.strip(":")
        if trigger != f":{typo}:" or not re.fullmatch("[a-z]+", typo):
            problem(trigger, "Not an ASCII whole-word trigger")
        if repeated_letter_change(typo, correction):
            problem(trigger, "Repeated-letter correction")
        if len(correction) > len(typo) and typo.replace("i", "") == correction.replace("i", ""):
            problem(trigger, "Missing-i correction")
        if typo in english:
            problem(trigger, "Valid English word (Webster/GCIDE word lists)")
        if typo in PROTECTED:
            problem(trigger, PROTECTED[typo])
        for language, dictionary in dictionaries.items():
            # QMK matches without regard to capitalization.
            if any(dictionary.lookup(form) for form in (typo, typo.title(), typo.upper())):
                problem(trigger, f"Accepted {language} word, inflection, name, or compound")
        for other, other_correction, _ in existing:
            if other == trigger:
                # Recurring personal entries are now also in the active source.
                if other_correction != correction:
                    problem(trigger, "Active rule has a different replacement")
                continue
            if trigger in other or other in trigger:
                problem(trigger, f"Overlaps existing trigger {other}")
        for other, _, _ in personal:
            if other != trigger and (trigger in other or other in trigger):
                problem(trigger, f"Overlaps personal trigger {other}")
        for other, _, _ in existing + personal:
            # A replacement should not be interpreted as another typo.
            if other in f":{correction}:":
                problem(trigger, f"Replacement matches trigger {other}")

    forms_checked = 0
    fragment_matches = {}
    for word in danish_forms(dictionaries["da_DK"]):
        forms_checked += 1
        # QMK treats DK_AE/DK_ARNG as boundaries and shifted DK_OSTR as
        # a boundary. Treat every non-ASCII character as a boundary here,
        # deliberately covering more cases than the firmware.
        for fragment in re.findall("[a-z]+", word.lower()):
            if fragment in bare and fragment != word.lower():
                fragment_matches.setdefault(fragment, word)
    for fragment, example in fragment_matches.items():
        problem(bare[fragment], f"Danish word fragment near a non-ASCII character: {example}")
    # Audit actual active trigger patterns, including legacy partial-word rules.
    # Affix generation deliberately overgenerates. Findings for legacy rules
    # are informational: all original rules are retained at the user's request.
    active_problems = {}
    matcher = ahocorasick.Automaton()
    for trigger, correction, _ in existing:
        matcher.add_word(trigger, trigger)
        typo = trigger.strip(":")
        languages = [language for language, dictionary in dictionaries.items()
                     if any(dictionary.lookup(form) for form in (typo, typo.title(), typo.upper()))]
        if typo in english or typo in PROTECTED or languages:
            active_problems[trigger] = {"reason": "Valid/recognized word, name, inflection or compound", "languages": languages}
    matcher.make_automaton()
    def check_active_word(word, language):
        projected = ":" + re.sub("[^a-z]", ":", word.lower()) + ":"
        for _, trigger in matcher.iter(projected):
            active_problems.setdefault(trigger, {"reason": "Trigger matches a dictionary word", "language": language, "example": word})

    for word in sorted(english):
        check_active_word(word, "English")
    active_forms_checked = {}
    for language, dictionary in dictionaries.items():
        count = 0
        for word in danish_forms(dictionary):
            check_active_word(word, language)
            count += 1
        active_forms_checked[language] = count
    return {
        "personal_entries": len(personal),
        "english_word_list_size": len(english),
        "hunspell_roots": {language: len(dictionary.dic.words) for language, dictionary in dictionaries.items()},
        "danish_forms_checked_including_overgeneration": forms_checked,
        "problems": problems,
        "active_entries": len(existing),
        "active_affix_forms_checked": active_forms_checked,
        "active_problems": active_problems,
        "sources": sources,
        "dependency_versions": {package: version(package) for package in ("spylls", "english-words", "pyahocorasick")},
        "limitations": [
            "Dictionary checks cannot cover every proper name, identifier, rare word, or newly formed Danish compound.",
            "Personal candidates and the active source are checked; generated firmware and physical host behavior still need appropriate validation.",
        ],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cache", required=True, type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    report = audit(args.cache)
    if args.report:
        args.report.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2, ensure_ascii=True))
    return bool(report["problems"])


if __name__ == "__main__":
    raise SystemExit(main())
