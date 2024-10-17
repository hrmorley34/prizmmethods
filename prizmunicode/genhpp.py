from collections import defaultdict

from .charmap import BYTE_MAP
from .searchmap import (
    JUMP_RECURSECOUNT,
    JUMP_SPACE,
    JUMP_STOPCOUNT,
    JUMPCHARS,
    SORT_ASCII_PTR_MAP,
    SORT_BYTE_MAP,
)

__all__ = [
    "create_cpp_searchconvert",
    "create_cpp_mbstartcheck",
    "main",
]

INVALID_C_CHAR = set(map(ord, "'\\"))


def get_c_char(i: int, allow_ascii: bool = False):
    assert 0x00 <= i <= 0xFF
    if allow_ascii and 0x20 <= i < 0x7F and i not in INVALID_C_CHAR:
        return f"'{chr(i)}'"
    return f"'\\x{i:02x}'"


CPP_DEFAULT_CHAR = " "
CPP_DEFAULT_C_CHAR = get_c_char(ord(CPP_DEFAULT_CHAR), True)
CPP_SEARCH_DEFAULT = JUMPCHARS.index(JUMP_SPACE)

CPP_SWITCHED_TOP = """\
{ret} {fname}(const MBChar *&c)
{{
    if (*c == '\\0')
        return {null};
    switch (*c++)
    {{"""
CPP_CASE_LINE = """\
        case {src}:"""
CPP_CASE_RETURN = """\
            return {dest};"""
CPP_NESTED_TOP = """\
        case {src}:
            switch (*c++)
            {{"""
CPP_NESTED_CASE_LINE = """\
                case {src}:"""
CPP_NESTED_CASE_RETURN = """\
                    return {dest};"""
CPP_NESTED_BOTTOM = """\
                default:
                    return {default};
            }}"""
CPP_SWITCHED_BOTTOM = """\
        default:
            return {default};
    }}
}}
"""


def create_cpp_searchconvert(fname: str) -> str:
    returnvals: defaultdict[str, set[int]] = defaultdict(set)
    groups: dict[int, dict[int, str]] = {}
    for i1, c1 in SORT_BYTE_MAP.items():
        if isinstance(c1, dict):
            groups[i1] = c1
        else:
            returnvals[c1].add(i1)

    elements: list[str] = [
        CPP_SWITCHED_TOP.format(fname=fname, ret="NonMBChar", null="'\\0'")
    ]
    for c in sorted(returnvals):
        iset = returnvals[c]
        if not iset:
            continue
        if c == CPP_DEFAULT_CHAR:  # covered by default case
            continue
        for i in sorted(iset):
            elements.append(CPP_CASE_LINE.format(src=get_c_char(i, True)))
        elements.append(CPP_CASE_RETURN.format(dest=get_c_char(ord(c), True)))

    for i1 in sorted(groups):
        c1 = groups[i1]
        returnvals.clear()

        for i2, c2 in c1.items():
            returnvals[c2].add(i2)

        elements.append(CPP_NESTED_TOP.format(src=get_c_char(i1)))
        for c in sorted(returnvals):
            iset = returnvals[c]
            if not iset:
                continue
            if c == CPP_DEFAULT_CHAR:  # covered by default case
                continue
            for i in sorted(iset):
                elements.append(CPP_NESTED_CASE_LINE.format(src=get_c_char(i)))
            elements.append(
                CPP_NESTED_CASE_RETURN.format(dest=get_c_char(ord(c), True))
            )
        elements.append(CPP_NESTED_BOTTOM.format(default=CPP_DEFAULT_C_CHAR))

    elements.append(CPP_SWITCHED_BOTTOM.format(default=CPP_DEFAULT_C_CHAR))
    return "\n".join(elements)


def create_cpp_searchptrconvert(fname: str) -> str:
    returnvals: defaultdict[int, set[int]] = defaultdict(set)
    for i1, c1 in SORT_ASCII_PTR_MAP.items():
        returnvals[c1].add(i1)

    elements: list[str] = [
        CPP_SWITCHED_TOP.format(fname=fname, ret="SearchIndex", null="-1")
    ]
    for c in sorted(returnvals):
        iset = returnvals[c]
        if not iset:
            continue
        if c == CPP_SEARCH_DEFAULT:  # covered by default case
            continue
        for i in sorted(iset):
            elements.append(CPP_CASE_LINE.format(src=get_c_char(i, True)))
        elements.append(CPP_CASE_RETURN.format(dest=c))

    elements.append(CPP_SWITCHED_BOTTOM.format(default=CPP_SEARCH_DEFAULT))
    return "\n".join(elements)


CPP_MB_TOP = """\
bool {fname}(const MBChar c)
{{
    switch (c)
    {{"""
CPP_MB_BOTTOM = """\
        default:
            return false;
    }
}
"""


def create_cpp_mbstartcheck(fname: str) -> str:
    elements: list[str] = [CPP_MB_TOP.format(fname=fname)]
    gen_lines = False
    for i in sorted(BYTE_MAP):
        if isinstance(BYTE_MAP[i], dict):
            gen_lines = True
            elements.append(CPP_CASE_LINE.format(src=get_c_char(i)))
    if gen_lines:
        elements.append(CPP_CASE_RETURN.format(dest="true"))
    elements.append(CPP_MB_BOTTOM)
    return "\n".join(elements)


def create_cpp_jumpcharcount(cname: str) -> str:
    return f"""\
const int {cname} = {len(JUMPCHARS)};
"""


def create_cpp_jumplayersize(fname: str) -> str:
    return f"""\
int {fname}(const int depth)
{{
    const int jump_stopcount = {JUMP_STOPCOUNT};
    const int jump_recursecount = {JUMP_RECURSECOUNT};

    int rd = 1;
    for (int i = 0; i < depth; i++)
        rd *= jump_recursecount;
    return jump_stopcount * (1 - rd) / (1 - jump_recursecount) + rd;
}}
"""


def create_cpp_isjumpstop(fname: str) -> str:
    indexes = [i for i, j in enumerate(JUMPCHARS) if not j.stop]
    cases = "\n".join(CPP_CASE_LINE.format(src=i) for i in indexes)
    if cases:
        cases += "\n" + CPP_CASE_RETURN.format(dest="false")
    return f"""\
bool {fname}(const SearchIndex i)
{{
    switch (i)
    {{
{cases}
        default:
            return true;
    }}
}}
"""


def main(argv: list[str]) -> None:
    if len(argv) == 3 and argv[1] == "create_hpp":
        with open(argv[2], "w") as f:
            f.write(
                "\n".join(
                    [
                        create_cpp_searchconvert("ReadSearchChar"),
                        create_cpp_searchptrconvert("ReadSearchCharPtr"),
                        # not required - syscall MB_IsLead has same function
                        # create_cpp_mbstartcheck(...),
                        create_cpp_jumpcharcount("jumpCharCount"),
                        create_cpp_jumplayersize("GetJumpDepth"),
                        create_cpp_isjumpstop("IsSearchStop"),
                    ]
                )
            )
    else:
        print(f"Usage: {argv[0]} SUBCMD ...")
        print("    create_hpp PATH")


if __name__ == "__main__":
    import sys

    main(sys.argv)
