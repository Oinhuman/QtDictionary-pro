from pathlib import Path
import re

# 所有词库输入输出固定在脚本同级 txt 目录，避免生成到项目根目录。
TXT_DIR = Path(__file__).resolve().parent / "txt"
SOURCE_FILE = TXT_DIR / "2.txt"
OUTPUT_FILE = TXT_DIR / "output_utf8_file.txt"
PHONETICS_FILE = TXT_DIR / "phonetics.txt"
WORDLIST_HEADER = "# QtDictionary wordlist v2"

# 匹配“英文 [音标] 中文释义”一行式词条；音标部分可省略。
INLINE_ENTRY_RE = re.compile(r"^([^\s\[]+)\s*(?:\[\s*([^\]]+?)\s*\]\s*)?(.*)$")


def load_phonetics():
    """读取外置音标表，key 使用 casefold 统一大小写。"""
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
    """按 UTF-8 BOM、GBK 顺序解码原始词库，最后用替换策略兜底。"""
    data = SOURCE_FILE.read_bytes()
    for encoding in ("utf-8-sig", "gbk"):
        try:
            return data.decode(encoding)
        except UnicodeDecodeError:
            continue
    return data.decode("utf-8", errors="replace")


def normalize_phonetic(phonetic):
    """规范音标格式：修正常见字符差异，并统一包裹为 /.../。"""
    phonetic = phonetic.strip()
    if not phonetic:
        return ""
    phonetic = phonetic.replace("∫", "ʃ")
    phonetic = phonetic.strip("[]/")
    return f"/{phonetic}/"


def is_heading(line):
    """识别章节标题或空行，解析词条时直接跳过。"""
    return (
        not line
        or line.startswith("(")
        or re.fullmatch(r"[A-Z]", line) is not None
        or " " not in line
    )


def parse_inline_entries(lines, phonetics):
    """解析一行式词条，优先使用行内音标，缺失时查外置音标表。"""
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
    """解析旧版两行式词条：英文一行、中文一行。"""
    entries = []
    for i in range(0, len(lines) - 1, 2):
        english = lines[i].strip()
        chinese = lines[i + 1].strip()
        if not english:
            continue
        entries.append((english, normalize_phonetic(phonetics.get(english.casefold(), "")), chinese))
    return entries


def merge_entries(entries):
    """合并重复英文词条：补齐音标，并用分号拼接不同释义。"""
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
    """生成 v2 词库内容：文件头 + 英文/音标/中文三行一组。"""
    output_lines = [WORDLIST_HEADER]
    for english, phonetic, chinese in merge_entries(entries):
        output_lines.extend([english, phonetic, chinese])
    return "\n".join(output_lines) + "\n"


# 主流程：已是 v2 时原样输出；否则解析并升级为 v2。
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
