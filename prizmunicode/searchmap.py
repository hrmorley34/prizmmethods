import string
import unicodedata
from dataclasses import dataclass
from functools import lru_cache
from typing import Generator

from .charmap import BYTE_MAP, CHAR_MAP, Alias

__all__ = [
    "JumpChar",
    "JUMP_SPACE",
    "JUMP_DIGIT",
    "JUMP_ASCII",
    "JUMPCHARS",
    "JUMP_STOPCOUNT",
    "JUMP_RECURSECOUNT",
    "jump_layersize",
    "get_jump_index",
    "SearchKeys",
    "try_map_searchchar",
    "try_map_searchchar_index",
    "try_map_searchstring",
    "get_title_cats",
    "SORT_BYTE_MAP",
    "SORT_ASCII_PTR_MAP",
]


@dataclass(frozen=True)
class JumpChar:
    char: str
    stop: bool


JUMP_SPACE = JumpChar("", True)
JUMP_DIGIT = JumpChar("0", True)
JUMP_ASCII = {c: JumpChar(c, False) for c in string.ascii_uppercase}
JUMPCHARS = [
    JUMP_SPACE,
    JUMP_DIGIT,
    *(JUMP_ASCII[c] for c in string.ascii_uppercase),
]

JUMP_STOPCOUNT = sum(j.stop for j in JUMPCHARS)
JUMP_RECURSECOUNT = len(JUMPCHARS) - JUMP_STOPCOUNT


def jump_layersize(depth: int) -> int:
    assert depth >= 0, f"Bad depth {depth}"
    # e.g. for depth 3:
    # stop + recurse * (stop + recurse * (stop + recurse.))
    # = stop + (recurse * stop) + (recurse^2 * stop) + recurse^3
    # = stop * (1 + recurse + recurse^2) + recurse^3
    # = stop * (1 - recurse^(2+1))/(1 - recurse) + recurse^3, by the geometric series
    rd = JUMP_RECURSECOUNT**depth
    return JUMP_STOPCOUNT * (1 - rd) // (1 - JUMP_RECURSECOUNT) + rd


def get_jump_index(jumpkey: tuple[JumpChar, ...], pointerdepth: int) -> int:
    pindex = 0
    for kindex, k in enumerate(jumpkey):
        depth = pointerdepth - kindex - 1
        if depth < 0:
            break
        layersize = jump_layersize(depth)
        for jumpchar in JUMPCHARS:
            if k == jumpchar:
                if k.stop:
                    return pindex
                break
            else:
                # shifts following index (the one we're searching for)
                pindex += 1 if jumpchar.stop else layersize
    return pindex


@dataclass(frozen=True)
class SearchKeys:
    sortkey: str
    jumpkey: tuple[JumpChar, ...]

    @property
    def jumpstop(self) -> bool:
        return len(self.jumpkey) > 0 and self.jumpkey[-1].stop

    def __post_init__(self) -> None:
        assert all(j in JUMPCHARS for j in self.jumpkey)
        assert all(not j.stop for j in self.jumpkey[:-1])

    def __add__(self, r2: "SearchKeys") -> "SearchKeys":
        assert isinstance(r2, SearchKeys)
        return SearchKeys(
            sortkey=self.sortkey + r2.sortkey,
            jumpkey=self.jumpkey if self.jumpstop else self.jumpkey + r2.jumpkey,
        )


SEARCH_SPACE = SearchKeys(" ", (JUMP_SPACE,))

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
    sk = SearchKeys("", ())
    for index, normchar in enumerate(norm):
        if normchar in string.ascii_uppercase:
            sk += SearchKeys(normchar, (JUMP_ASCII[normchar],))
        elif normchar in string.digits:
            sk += SearchKeys(normchar, (JUMP_DIGIT,))
        elif index and unicodedata.category(normchar)[0] not in "LNPSZ":
            continue
        else:
            if verbose:
                print(f"Warning: search skipping {char!r}.")
            return None
    return sk


def try_map_searchchar_index(char: str, *, verbose: bool = True) -> int | None:
    sk = try_map_searchchar(char, verbose=verbose)
    if sk is None or not sk.jumpkey:
        return None
    return JUMPCHARS.index(sk.jumpkey[0])


def try_map_searchstring(s: str) -> SearchKeys | None:
    initial = SearchKeys("", ())
    for char in s:
        r = try_map_searchchar(char)
        if r is None:
            return None
        initial += r
    return initial


def get_title_cats(depth: int) -> Generator[str, None, None]:
    if depth <= 0:
        yield ""
        return
    for char in JUMPCHARS:
        if char.stop or depth == 1:
            yield char.char
        else:
            for suffix in get_title_cats(depth - 1):
                yield char.char + suffix


SORT_BYTE_MAP: dict[int, str | dict[int, str]] = {}
SORT_ASCII_PTR_MAP: dict[int, int] = {}
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
        if 0x20 <= i1 < 0x7F:
            s = try_map_searchchar_index(c1, verbose=False)
            if s is not None:
                SORT_ASCII_PTR_MAP[i1] = s
