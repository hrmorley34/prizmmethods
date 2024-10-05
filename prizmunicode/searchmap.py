import string
import unicodedata
from dataclasses import dataclass
from functools import lru_cache
from typing import Generator

from .charmap import BYTE_MAP, CHAR_MAP, Alias

__all__ = [
    "SearchKeys",
    "JUMP_SPACE",
    "JUMP_DIGIT",
    "try_map_searchchar",
    "try_map_searchstring",
    "get_title_cats",
    "SORT_BYTE_MAP",
]


@dataclass(frozen=True)
class SearchKeys:
    sortkey: str
    jumpkey: str
    jumpstop: bool = False

    def __add__(self, r2: "SearchKeys") -> "SearchKeys":
        assert isinstance(r2, SearchKeys)
        return SearchKeys(
            sortkey=self.sortkey + r2.sortkey,
            jumpkey=self.jumpkey if self.jumpstop else self.jumpkey + r2.jumpkey,
            jumpstop=self.jumpstop or r2.jumpstop,
        )


JUMP_SPACE = ""
JUMP_DIGIT = "0"
SEARCH_SPACE = SearchKeys(" ", JUMP_SPACE, jumpstop=True)

UNICODE_CONVERSIONS = {"Ø": "O", "ø": "o"}


@lru_cache()
def try_map_searchchar(char: str, *, verbose: bool = True) -> SearchKeys | None:
    if len(char) == 1:
        cat, _ = unicodedata.category(char)
        if cat in "PSZ":  # punctuation, symbols, separators
            return SEARCH_SPACE

    norm = unicodedata.normalize("NFKD", char.casefold()).upper()
    norm = "".join(UNICODE_CONVERSIONS.get(c, c) for c in norm)
    if char in CHAR_MAP:
        norm = norm[0]
    sk = SearchKeys("", "")
    for index, normchar in enumerate(norm):
        if normchar in string.ascii_uppercase:
            sk += SearchKeys(normchar, normchar)
        elif normchar in string.digits:
            sk += SearchKeys(normchar, JUMP_DIGIT, jumpstop=True)
        elif index and unicodedata.category(normchar)[0] not in "LNPSZ":
            continue
        else:
            if verbose:
                print(f"Warning: search skipping {char!r}.")
            return None
    return sk


def try_map_searchstring(s: str) -> SearchKeys | None:
    initial = SearchKeys("", "")
    for char in s:
        r = try_map_searchchar(char)
        if r is None:
            return None
        initial += r
    return initial


def get_title_cats(depth: int) -> Generator[str, None, None]:
    yield ""
    if depth <= 0:
        return
    yield "0"
    if depth == 1:
        yield from string.ascii_uppercase
        return
    for c in string.ascii_uppercase:
        for suffix in get_title_cats(depth - 1):
            yield c + suffix


SORT_BYTE_MAP: dict[int, str | dict[int, str]] = {}
for i1, c1 in BYTE_MAP.items():
    if isinstance(c1, dict):
        sub_byte_map: dict[int, str] = {}
        SORT_BYTE_MAP[i1] = sub_byte_map
        for i2, c2 in c1.items():
            if isinstance(c2, Alias):
                c2 = c2.char
            c = try_map_searchchar(c2, verbose=False)
            if c is not None:
                sub_byte_map[i2] = c.sortkey
    else:
        if isinstance(c1, Alias):
            c1 = c1.char
        c = try_map_searchchar(c1, verbose=False)
        if c is not None:
            SORT_BYTE_MAP[i1] = c.sortkey
