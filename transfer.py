from pathlib import Path
import re

TXT_DIR = Path(__file__).resolve().parent / "txt"
SOURCE_FILE = TXT_DIR / "2.txt"
OUTPUT_FILE = TXT_DIR / "output_utf8_file.txt"
PHONETICS_FILE = TXT_DIR / "phonetics.txt"
WORDLIST_HEADER = "# QtDictionary wordlist v2"
INLINE_ENTRY_RE = re.compile(r"^([^\s\[]+)\s*(?:\[\s*([^\]]+?)\s*\]\s*)?(.*)$")


def load_phonetics():
    phonetics = {}
    if not PHONETICS_FILE.exists():
        return phonetics

    for line in PHONETICS_FILE.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split("\t", 1)
        if len(parts) != 2:
            continue
        word, phonetic = parts
        phonetics[word.casefold()] = phonetic.strip()
    return phonetics


def read_source_text():
    data = SOURCE_FILE.read_bytes()
    for encoding in ("utf-8-sig", "gbk"):
        try:
            return data.decode(encoding)
        except UnicodeDecodeError:
            continue
    return data.decode("utf-8", errors="replace")


def normalize_phonetic(phonetic):
    phonetic = phonetic.strip()
    if not phonetic:
        return ""
    phonetic = phonetic.replace("∫", "ʃ")
    phonetic = phonetic.strip("[]/")
    return f"/{phonetic}/"


def is_heading(line):
    return (
        not line
        or line.startswith("(")
        or re.fullmatch(r"[A-Z]", line) is not None
        or " " not in line
    )


def parse_inline_entries(lines, phonetics):
    entries = []
    seen_phonetics = {}
    for raw_line in lines:
        line = raw_line.lstrip("\ufeff").strip()
        if is_heading(line):
            continue

        match = INLINE_ENTRY_RE.match(line)
        if not match:
            continue

        english = match.group(1).strip()
        phonetic = normalize_phonetic(match.group(2) or "")
        chinese = match.group(3).strip()
        if not english or not chinese:
            continue

        key = english.casefold()
        if not phonetic:
            phonetic = seen_phonetics.get(key, "") or normalize_phonetic(phonetics.get(key, ""))
        if phonetic:
            seen_phonetics[key] = phonetic

        entries.append((english, phonetic, chinese))
    return entries


def parse_pair_entries(lines, phonetics):
    entries = []
    for i in range(0, len(lines) - 1, 2):
        english = lines[i].strip()
        chinese = lines[i + 1].strip()
        if not english:
            continue
        entries.append((english, normalize_phonetic(phonetics.get(english.casefold(), "")), chinese))
    return entries


def merge_entries(entries):
    merged = []
    indexes = {}
    for english, phonetic, chinese in entries:
        key = english.casefold()
        if key not in indexes:
            indexes[key] = len(merged)
            merged.append([english, phonetic, chinese])
            continue

        current = merged[indexes[key]]
        if phonetic and not current[1]:
            current[1] = phonetic
        if chinese and chinese not in current[2].split("；"):
            current[2] = f"{current[2]}；{chinese}" if current[2] else chinese
    return merged


def build_v2_content(entries):
    output_lines = [WORDLIST_HEADER]
    for english, phonetic, chinese in merge_entries(entries):
        output_lines.extend([english, phonetic, chinese])
    return "\n".join(output_lines) + "\n"


phonetics = load_phonetics()
source_content = read_source_text()
lines = source_content.splitlines()
if lines and lines[0].strip() == WORDLIST_HEADER:
    output_content = source_content
else:
    inline_entries = parse_inline_entries(lines, phonetics)
    entries = inline_entries if inline_entries else parse_pair_entries(lines, phonetics)
    output_content = build_v2_content(entries)

OUTPUT_FILE.write_text(output_content, encoding="utf-8")

print(f"文件转换完成：{OUTPUT_FILE}")
