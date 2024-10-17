import bisect
import re
import struct
import sys
import zipfile
from abc import ABC, abstractmethod
from collections import defaultdict
from dataclasses import dataclass
from itertools import count
from math import log
from typing import IO, Any, Callable, Generator, Iterable

import lxml.etree

from prizmunicode.charmap import try_map_string
from prizmunicode.searchmap import (
    JUMPCHARS,
    get_title_cats,
    jump_layersize,
    try_map_searchstring,
)

MAX_METHOD_TITLE_LENGTH = 128
MAX_PLACE_NOTATION_LENGTH = 256

BELLS = "1234567890ETABCDFGHJKLMNPQRSUVWYZ"
PN_CROSS = {"-", "X"}
PN_DOT = {"."}
assert len(BELLS) == 33


def convert_pn_part(pn: str) -> list[int]:
    cur_val = 0
    pn_out: list[int] = []
    for char in pn:
        if char in PN_CROSS:
            if cur_val:
                pn_out.append(cur_val)
                cur_val = 0
            pn_out.append(0)
        elif char in PN_DOT:
            if cur_val:
                pn_out.append(cur_val)
                cur_val = 0
        else:
            i = BELLS.index(char)
            cur_val |= 1 << i
    if cur_val:
        pn_out.append(cur_val)
    return pn_out


def convert_pn(pn: str) -> list[int]:
    pn = pn.upper()
    palindromes = pn.split(",")
    if len(palindromes) > 1:
        out_pn: list[int] = []
        for part in palindromes:
            out_part = convert_pn_part(part)
            out_pn.extend(out_part)
            out_pn.extend(out_part[-2::-1])
        return out_pn
    else:
        return convert_pn_part(palindromes[0])


class Row:
    stage: int
    row: dict[int, int]  # 0-indexed

    def __init__(self, row: tuple[int, ...] | str) -> None:
        self.stage = len(row)
        if isinstance(row, str):
            row = tuple(BELLS.index(c) for c in row.upper())
        self.row = dict(zip(count(), row))

    def __mul__(self, r: "Row") -> "Row":
        assert isinstance(r, Row)
        assert self.stage == r.stage
        return Row(tuple(self.row[r.row[i]] for i in range(self.stage)))

    def to_tuple(self) -> tuple[int, ...]:
        return tuple(self.row[i] for i in range(self.stage))

    def to_str(self) -> str:
        return "".join(BELLS[b] for b in self.to_tuple())

    def __repr__(self) -> str:
        return f"Row({self.to_str()!r})"

    def is_rounds(self) -> bool:
        return all(self.row[i] == i for i in range(self.stage))

    def get_unchanged(self) -> list[int]:
        return [i for i in range(self.stage) if self.row[i] == i]


@dataclass(init=True, repr=True, order=False)
class Method:
    stage: int
    original_title: str
    raw_title: bytes
    sort_title: str
    pn: list[int]
    leadcount: int
    huntbells: int

    def dumps(self) -> bytes:
        titlelength = len(self.raw_title)
        if titlelength + 1 > MAX_METHOD_TITLE_LENGTH:  # +1 for null terminator
            raise ValueError("Method title too long")
        leadlength = len(self.pn)
        if leadlength > MAX_PLACE_NOTATION_LENGTH:
            raise ValueError("Place notation too long")

        packer = struct.Struct(f"< B {titlelength+1}s H {leadlength}H H H")

        data = packer.pack(
            titlelength,
            self.raw_title + b"\0",
            leadlength,
            *self.pn,
            self.leadcount,
            self.huntbells,
        )
        return struct.pack("<H", len(data)) + data

    def dump(self, f: IO[bytes]) -> int:
        return f.write(self.dumps())

    def __lt__(self, m: "Method") -> bool:
        assert isinstance(m, Method)
        # return self.inter_sort_title < m.inter_sort_title
        return self.sort_title < m.sort_title


FILE_VERSION = 0x02
MAGIC_WORD = b"CCML"
HEADER_STRUCT = struct.Struct("< 4s B B x B")
POINTERS_START = HEADER_STRUCT.size


class MethodFile:
    stage: int
    pointerdepth: int
    pointers: list[int]

    def __init__(self, stage: int, pointerdepth: int) -> None:
        self.stage = stage
        self.pointerdepth = pointerdepth
        self.pointers = [0] * jump_layersize(pointerdepth)

    def header_dumps(self) -> bytes:
        return HEADER_STRUCT.pack(
            MAGIC_WORD,
            FILE_VERSION,
            self.stage,
            self.pointerdepth,
        )

    def header_dump(self, f: IO[bytes]) -> int:
        if f.seek(0) != 0:
            raise Exception
        return f.write(self.header_dumps())

    def after_pointers(self) -> int:
        return POINTERS_START + struct.calcsize(f"< {len(self.pointers)}L")

    def pointers_dumps(self) -> bytes:
        return struct.pack(f"< {len(self.pointers)}L", *self.pointers)

    def pointers_dump(self, f: IO[bytes]) -> int:
        if f.seek(POINTERS_START) != POINTERS_START:
            raise Exception
        return f.write(self.pointers_dumps())

    def get_title_sort_checks(self) -> Generator[tuple[int, str], None, None]:
        yield from enumerate(get_title_cats(self.pointerdepth))
        yield (len(self.pointers), "\U0010ffff")

    def dump_methods(self, f: IO[bytes], sorted_methods: list[Method]) -> int:
        if f.seek(self.after_pointers()) != self.after_pointers():
            raise Exception
        f.truncate()

        title_sort_it = self.get_title_sort_checks()
        next_title_sort = next(title_sort_it)

        length_written = 0

        for method in sorted_methods:
            while next_title_sort[1] < method.sort_title:
                self.pointers[next_title_sort[0]] = f.tell()
                next_title_sort = next(title_sort_it)

            assert method.stage == self.stage
            length_written += method.dump(f)

        # Make remaining pointers point to EOF here
        while next_title_sort[0] < len(self.pointers):
            self.pointers[next_title_sort[0]] = f.tell()
            next_title_sort = next(title_sort_it)

        return length_written

    def dump(self, f: IO[bytes], sorted_methods: list[Method]) -> int:
        length = self.header_dump(f)
        length += self.pointers_dump(f)
        length += self.dump_methods(f, sorted_methods)
        assert f.tell() == length
        self.pointers_dump(f)  # rewrite after pointers updated in dump_methods
        f.seek(length)
        return length


SCHEMA = "http://www.cccbr.org.uk/methods/schemas/2007/05/methods"
TSCHEMA = "{" + SCHEMA + "}"


def find_property(element: Any, xpath: str) -> Any:
    p = element.find(f"./{xpath}")
    if p is None:
        p = element.find(f"../{TSCHEMA}properties/{xpath}")
    return p


def read_method(element: Any) -> Method:
    assert element.tag == f"{TSCHEMA}method"
    title = str(element.find(f"./{TSCHEMA}title").text)
    notation = str(element.find(f"./{TSCHEMA}notation").text)
    stage = int(find_property(element, f"{TSCHEMA}stage").text)
    leadHead = find_property(element, f"{TSCHEMA}leadHead")
    numberOfHunts = find_property(element, f"{TSCHEMA}numberOfHunts")

    lhrow = Row(str(leadHead.text))
    r = lhrow
    leadcount = 1
    while not r.is_rounds():
        r *= lhrow
        leadcount += 1

    huntbell_positions = lhrow.get_unchanged()
    if numberOfHunts is None:
        huntbell_count = 0
    else:
        huntbell_count = int(numberOfHunts.text)
    assert len(huntbell_positions) == huntbell_count
    huntbells = 0
    for bell in huntbell_positions:
        huntbells |= 1 << bell

    raw_title = try_map_string(title)
    assert raw_title is not None
    search_title = try_map_searchstring(title)
    assert search_title is not None
    return Method(
        stage=stage,
        original_title=title,
        raw_title=raw_title,
        sort_title=search_title.sortkey,
        pn=convert_pn(notation),
        leadcount=leadcount,
        huntbells=huntbells,
    )


class BaseFilterFunc(ABC):
    @abstractmethod
    def __call__(self, element: Any) -> bool:
        return True

    def __and__(self, e: Callable[[Any], bool]) -> "MultiFilterFunc":
        return MultiFilterFunc([self, e])

    def __rand__(self, e: Callable[[Any], bool]) -> "MultiFilterFunc":
        return MultiFilterFunc([e, self])


class FilterFunc(BaseFilterFunc):
    f: Callable[[Any], bool]

    def __init__(self, f: Callable[[Any], bool]) -> None:
        super().__init__()
        self.f = f

    def __call__(self, element: Any) -> bool:
        return self.f(element)


class MultiFilterFunc(BaseFilterFunc):
    fns: tuple[Callable[[Any], bool], ...]

    def __init__(self, fns: Iterable[Callable[[Any], bool]]) -> None:
        super().__init__()
        self.fns = tuple(fns)
        assert all(map(callable, self.fns))

    def __call__(self, element: Any) -> bool:
        return all(fn(element) for fn in self.fns)

    def __and__(self, e: Callable[[Any], bool]) -> "MultiFilterFunc":
        if isinstance(e, MultiFilterFunc):
            return MultiFilterFunc([*self.fns, *e.fns])
        return MultiFilterFunc([*self.fns, e])

    def __rand__(self, e: Callable[[Any], bool]) -> "MultiFilterFunc":
        return MultiFilterFunc([e, *self.fns])


class TitleFilter(BaseFilterFunc):
    title: tuple[str, ...] | re.Pattern[str]

    def __init__(self, title: str | Iterable[str] | re.Pattern[str]) -> None:
        super().__init__()
        if isinstance(title, str):
            self.title = (title,)
        elif isinstance(title, re.Pattern):
            self.title = title
        else:
            self.title = tuple(title)

    def __call__(self, element: Any) -> bool:
        title = element.find(f"./{TSCHEMA}title").text
        if isinstance(self.title, re.Pattern):
            return bool(self.title.match(title))
        # also try excluding stage name
        return title in self.title or " ".join(title.split()[:-1]) in self.title


class StageFilter(BaseFilterFunc):
    min: int | None
    max: int | None

    def __init__(self, min: int | None = None, max: int | None = None) -> None:
        super().__init__()
        self.min = min
        self.max = max

    def __call__(self, element: Any) -> bool:
        stage = int(find_property(element, f"{TSCHEMA}stage").text)
        if self.min is not None and stage < self.min:
            return False
        if self.max is not None and stage > self.max:
            return False
        return True


VALID_CLASSES = (
    None,
    "Place",
    "Bob",
    "Slow Course",
    "Treble Bob",
    "Delight",
    "Surprise",
    "Alliance",
    "Treble Place",
    "Hybrid",
)
XML_BOOLEANS = {"true": True, "false": False, "1": True, "0": False}


class ClassificationFilter(BaseFilterFunc):
    class_: tuple[str | None, ...] | None
    attrib: dict[str, bool]

    def __init__(
        self,
        class_: str | Iterable[str | None] | None = None,
        little: bool | None = None,
        differential: bool | None = None,
        plain: bool | None = None,
        trebleDodging: bool | None = None,
    ) -> None:
        super().__init__()
        if class_ is None:
            self.class_ = None
        elif isinstance(class_, str):
            self.class_ = (class_,)
        else:
            self.class_ = tuple(class_)
        self.attrib = {}
        if little is not None:
            self.attrib["little"] = little
        if differential is not None:
            self.attrib["differential"] = differential
        if plain is not None:
            self.attrib["plain"] = plain
        if trebleDodging is not None:
            self.attrib["trebleDodging"] = trebleDodging

    def __call__(self, element: Any) -> bool:
        classification = find_property(element, f"{TSCHEMA}classification")
        assert classification.text in VALID_CLASSES
        if self.class_ is not None and classification.text not in self.class_:
            return False
        for k, b in self.attrib.items():
            if XML_BOOLEANS[classification.get(k, "false")] != b:
                return False
        return True


def read_methods(
    file: IO[bytes], filter: Callable[[Any], bool] | None = None
) -> Generator[Method, None, None]:
    for _, element in lxml.etree.iterparse(
        file,
        events=("end",),
        tag=(f"{TSCHEMA}method", f"{TSCHEMA}methodSet", f"{TSCHEMA}collection"),
    ):
        if element.tag == f"{TSCHEMA}method" and (filter is None or filter(element)):
            yield read_method(element)
        element.clear()  # reduce memory usage


def read_methods_from_zip(
    filename: str, filter: Callable[[Any], bool] | None = None
) -> Generator[Method, None, None]:
    with zipfile.ZipFile(filename, "r") as z:
        with z.open(z.filelist[0], "r") as f:
            yield from read_methods(f, filter)


def group_and_sort_methods(*its: Iterable[Method]) -> dict[int, list[Method]]:
    d: defaultdict[int, list[Method]] = defaultdict(list)
    for it in its:
        for m in it:
            bisect.insort(d[m.stage], m)
    return dict(d)


DEFAULT_LIB = {
    "Original",
    "Plain Bob",
    "Reverse Bob",
    "Double Bob",
    "Little Bob",
    "Grandsire",
    "Stedman",
    "Reverse Stedman",
    "Erin",
    "Gainsborough Little Bob",
    "St Clement's College Bob",
    "Bastow Little Bob",
    "Bistow Little Bob",
    "St Martin's Bob",
    "St Simon's Bob",
    # Treble Bob:
    "Beverly Surprise",
    "Bourne Surprise",
    "Bristol Surprise",
    "Cambridge Surprise",
    "Ipswich Surprise",
    "Kent Treble Bob",
    "London Surprise",
    "Norfolk Surprise",
    "Norwich Surprise",
    "Oxford Treble Bob",
    "Primrose Surprise",
    "Superlative Surprise",
    "York Surprise",
    "Yorkshire Surprise",
    # Doubles:
    "Ashford Little Bob Doubles",
    "Blaisdon Bob Doubles",
    "Reverse Canterbury Pleasure Place Doubles",
    # Minor:
    "Double Dunkirk Bob Minor",
    "Single Oxford Bob Minor",
    "Double Oxford Bob Minor",
}
OUT_FILE = "methods/methods-{}.ccml"
OUT_FILE_CHARS = (
    None,
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "E",
    "T",
    "A",
    "B",
    "C",
    "D",
)

if __name__ == "__main__":
    assert len(sys.argv) == 2, "Takes one argument: path to CCCBR_methods.xml.zip"
    FILENAME = sys.argv[1]

    mfilter = StageFilter(2, 16)
    # mfilter &= TitleFilter(DEFAULT_LIB)
    mit = read_methods_from_zip(FILENAME, mfilter)
    ms = group_and_sort_methods(mit)
    for stage, methods in sorted(ms.items(), key=lambda t: t[0]):
        out_file = OUT_FILE.format(OUT_FILE_CHARS[stage])
        print(f"{stage}: Writing to {out_file}")

        # aim for an average of 10 methods per bucket
        # (likely to be a lot more for some letters)
        float_pointerdepth = log(len(methods) / 10.0, len(JUMPCHARS))
        pointerdepth = max(int(float_pointerdepth), 0)
        # pointerdepth = min(pointerdepth, 1)
        print(f"{stage}: Using depth {pointerdepth}")

        with open(out_file, "wb") as f:
            MethodFile(stage, pointerdepth).dump(f, methods)
        print(f"Written {len(methods)} methods for {stage} bells.")
