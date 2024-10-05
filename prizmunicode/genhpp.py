from collections import defaultdict

from .charmap import BYTE_MAP
from .searchmap import SORT_BYTE_MAP

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

# TODO: Group same return values together for compiler
CPP_TOP = """\
char {fname}(const char *&c)
{{
    if (*c == '\\0')
        return '\\0';
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
CPP_NESTED_BOTTOM = f"""\
                default:
                    return {CPP_DEFAULT_C_CHAR};
            }}"""
CPP_BOTTOM = f"""\
        default:
            return {CPP_DEFAULT_C_CHAR};
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

    elements: list[str] = [CPP_TOP.format(fname=fname)]
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
        elements.append(CPP_NESTED_BOTTOM)

    elements.append(CPP_BOTTOM)
    return "\n".join(elements)


CPP_MB_TOP = """\
bool {fname}(const char c)
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


def main(argv: list[str]) -> None:
    if len(argv) == 3 and argv[1] == "create_hpp":
        with open(argv[2], "w") as f:
            f.write(
                "\n".join(
                    [
                        create_cpp_searchconvert("ReadSearchChar"),
                        # not required - syscall MB_IsLead has same function
                        # create_cpp_mbstartcheck(...),
                    ]
                )
            )
    else:
        print(f"Usage: {argv[0]} SUBCMD ...")
        print("    create_hpp PATH")


if __name__ == "__main__":
    import sys

    main(sys.argv)
