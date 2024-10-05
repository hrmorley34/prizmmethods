import string
import unicodedata
from dataclasses import dataclass
from functools import lru_cache
from itertools import groupby
from typing import Container, Generator, Iterable

__all__ = [
    "Alias",
    "CharSubDictElement",
    "CharSubDict",
    "CharDictElement",
    "CharDict",
    "AnyCharDict",
    "crawl_map",
    "BYTE_MAP",
    "CHAR_MAP",
    "try_map_char",
    "try_map_string",
]


@dataclass(frozen=True)
class Alias:
    char: str

    def __str__(self) -> str:
        return self.char

    def __repr__(self) -> str:
        return f"Alias({self.char!r})"


CharSubDictElement = str | Alias
CharSubDict = dict[int, CharSubDictElement]
CharDictElement = CharSubDictElement | CharSubDict
CharDict = dict[int, CharDictElement]
AnyCharDict = CharSubDict | CharDict


def set_char(d: CharDict | CharSubDict, index: int, char: CharSubDictElement):
    assert 0 <= index <= 0xFF
    assert index not in d
    assert str(char) == unicodedata.normalize("NFC", str(char)), char
    d[index] = char


def set_chars(d: CharDict, index: int, subd: CharSubDict):
    assert 0 <= index <= 0xFF
    assert all(0 <= i <= 0xFF for i in subd)
    if index not in d:
        d[index] = subd
        return
    child = d[index]
    assert isinstance(child, dict)
    assert all(i not in child for i in subd)
    child.update(subd)


def fill_chars(
    d: CharDict | CharSubDict,
    offset: int,
    s: Iterable[CharSubDictElement | None],
    *,
    ignore: Container[str] = {"_", " "},
):
    for index, char in enumerate(s, offset):
        if char is None or char in ignore:
            continue
        set_char(d, index, char)


def crawl_map(
    map: CharDict | CharSubDict | CharDictElement | CharSubDictElement,
    *,
    prefix: bytes = b"",
) -> Generator[tuple[bytes, CharSubDictElement], None, None]:
    if not isinstance(map, dict):
        yield (bytes(prefix), map)
        return
    for k, v in map.items():
        yield from crawl_map(v, prefix=prefix + bytes((k,)))


BYTE_MAP: CharDict = {}

fill_chars(
    BYTE_MAP,
    0x00,
    "_𝐟𝐩𝐧𝝁𝐦𝐤𝐌𝐆𝐓𝐏  ↵_ "
    "≤≠≥⇒______𝐀𝐁𝐂𝐃𝐄𝐅"
    " !\"#$%&'()*+,-./"
    "0123456789:;<=>?"
    "@ABCDEFGHIJKLMNO"
    "PQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmno"
    "pqrstuvwxyz{|}~_"
    "______√−____ ___"
    "𝑥___________°___"
    "_________×__ ___"
    "_____⏨___÷_  ___"
    "__ ȳ_______ ŷ𝐫𝜽_"
    "________ _______",
)
set_char(BYTE_MAP, 0x0B, Alias("𝐄"))
set_char(BYTE_MAP, 0x20, " ")
set_char(BYTE_MAP, 0x5F, "_")
set_char(BYTE_MAP, 0xC2, "x̄")  # x + combining bar
set_char(BYTE_MAP, 0xCB, "x̂")  # x + combining hat

set_chars(
    BYTE_MAP,
    0x7F,
    {
        0x50: "𝐢",
        0x53: "∞",
        0x54: "∠",
        0xC7: "p̂",  # p + combining hat
    },
)

BYTE_MAP_E5: CharSubDict = {}
BYTE_MAP_E6: CharSubDict = {}
BYTE_MAP[0xE5], BYTE_MAP[0xE6] = BYTE_MAP_E5, BYTE_MAP_E6


# Latin Extended-A / Greek / Cyrillic
E56_CASELESS = (
    "_ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎ"
    "ÏÐÑÒÓÔÕÖØÙÚÛÜÝÞ_"
    "ŸĂĄĆČŒĎĘĚŁŃŇŐŘŚŠ"
    "ŤŮŰŹŻŽ__________"
    "ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠ"
    "ΡΣ_ΤΥΦΧΨΩ_______"
    "АБВГДЕЁЖЗИЙКЛМНО"
    "ПРСТУФХЦЧШЩЪЫЬЭ_"
    "ЮЯЄ_____________"
)
fill_chars(BYTE_MAP_E5, 0x00, E56_CASELESS)
fill_chars(BYTE_MAP_E6, 0x00, E56_CASELESS.lower())
set_char(BYTE_MAP_E6, 0x1F, "ß")
set_char(BYTE_MAP_E6, 0x52, "ς")

# Symbols
SYMBOLS_E590 = (
    "¡¿€_…‘’“”¢£¤¥§©ª"
    "¬®º«»  ⋅___⁉‼☆  "
    "𝒆  𝐗𝐘         ±∓"
    "⁰¹²³⁴⁵⁶⁷⁸⁹ ⁺⁻   "
    "₀₁₂₃₄₅₆₇₈₉ ₊₋ ₙ "
    "♠♣♥♦ ___⇦⇨⇧⇩☜☞☝☟"
    "①②③④⑤⑥_____⑦⑧⑨__"
)
fill_chars(BYTE_MAP_E5, 0x90, SYMBOLS_E590)
set_char(BYTE_MAP_E5, 0xB1, Alias("𝐏"))
set_char(BYTE_MAP_E5, 0xB2, Alias("𝐫"))
set_char(BYTE_MAP_E5, 0xBD, Alias(","))
set_char(BYTE_MAP_E5, 0xCD, Alias("₀"))
set_char(BYTE_MAP_E5, 0xCE, Alias("₁"))
set_char(BYTE_MAP_E5, 0xCF, Alias("₂"))
set_char(BYTE_MAP_E5, 0xDF, Alias("³"))

SYMBOLS_E690 = (
    "←→↑↓↔↕↖↗↘↙◀▶▲▼▸▷"
    "   ○●□■◇◆☑ _△▽ ◁"
    "≒≈≡≢≅∼∝∬∮∂_∫∡∈∋⊆"
    "⊇⊂⊃∪∩∉∌⊈⊉⊄⊅∅∃∟∨∧"
    "∀⊕⊖⊗⊘⊥⇔∥∦⫽∇∴∵´˝_"
    "________________"
    "___________    _"
)
fill_chars(BYTE_MAP_E6, 0x90, SYMBOLS_E690)
set_char(BYTE_MAP_E6, 0xA1, Alias("["))
set_char(BYTE_MAP_E6, 0xA2, Alias("]"))
set_char(BYTE_MAP_E6, 0xFD, Alias("⏨"))

BYTE_MAP_E7: CharSubDict = {}
BYTE_MAP[0xE7] = BYTE_MAP_E7
# Small letters
fill_chars(BYTE_MAP_E7, 0x30, map(Alias, "₀₁₂₃₄₅₆₇₈₉"))
set_char(BYTE_MAP_E7, 0x61, "ₐ")
set_char(BYTE_MAP_E7, 0x65, "ₑ")
set_char(BYTE_MAP_E7, 0x68, "ₕ")
set_char(BYTE_MAP_E7, 0x6B, "ₖ")
set_char(BYTE_MAP_E7, 0x6C, "ₗ")
set_char(BYTE_MAP_E7, 0x6D, "ₘ")
set_char(BYTE_MAP_E7, 0x6E, Alias("ₙ"))
set_char(BYTE_MAP_E7, 0x6F, "ₒ")
set_char(BYTE_MAP_E7, 0x70, "ₚ")
set_char(BYTE_MAP_E7, 0x73, "ₛ")
set_char(BYTE_MAP_E7, 0x74, "ₜ")
set_char(BYTE_MAP_E7, 0x78, "ₓ")
# Italic letters
ITALICS = "_____ℎ__ 𝑎______" "𝑢____𝐹𝑒_𝑘_𝑅___𝜎_" "__𝑔__𝑡𝐺____𝑝𝜇𝑁𝐴𝐵"
fill_chars(BYTE_MAP_E7, 0x80, ITALICS)

# No known chars, but appears in MB_ElementCount
set_chars(BYTE_MAP, 0xF7, {})
set_chars(BYTE_MAP, 0xF9, {})


CHAR_MAP: dict[str, bytes] = {
    c: b for b, c in crawl_map(BYTE_MAP) if not isinstance(c, Alias)
}
assert all(c.char in CHAR_MAP for b, c in crawl_map(BYTE_MAP) if isinstance(c, Alias))

CHAR_MAP_BY_LENGTH: dict[int, dict[str, bytes]] = {
    k: dict(v)
    for k, v in groupby(
        sorted(CHAR_MAP.items(), key=lambda cb: len(cb[0])), key=lambda cb: len(cb[0])
    )
}
MAX_CHAR_MAP_LENGTH = max(CHAR_MAP_BY_LENGTH)


@lru_cache()
def try_map_char(char: str, *, verbose: bool = True) -> bytes | None:
    b = CHAR_MAP.get(char)
    if b is not None:
        return b

    norm = unicodedata.normalize("NFKD", char)
    normchar = norm[0]
    if normchar in string.ascii_letters + string.digits + string.punctuation:
        normconv = norm.encode("ascii", errors="ignore")
        if verbose:
            print(f"Warning: char normalising {char!r} -> {normconv!r}.")
        return normconv
    else:
        if verbose:
            print(f"Warning: char skipping {char!r}.")
        return None


def try_map_string(s: str) -> bytes | None:
    initial = bytes()
    index = 0
    while index < len(s):
        r = None
        for end in range(min(len(s), index + MAX_CHAR_MAP_LENGTH), index, -1):
            r = CHAR_MAP_BY_LENGTH.get(end - index, {}).get(s[index:end])
            if r is not None:
                index = end
                break
        if r is None:
            r = try_map_char(s[index])
            index += 1
        if r is None:
            return None
        initial += r
    return initial
