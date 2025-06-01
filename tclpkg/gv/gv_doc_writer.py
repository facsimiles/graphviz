#!/usr/bin/env python3

"""man page generator for language bindings"""

import dataclasses
import os
import re
import time
from pathlib import Path


@dataclasses.dataclass
class Syntax:
    """a language-specific syntax description"""

    nameprefix: str  # prefix of SWIG-generated functions
    paramstart: str  # start of a function declaration
    paramsep: str  # function parameter separator
    paramend: str  # end of a function declaration


@dataclasses.dataclass
class Lang:
    """template substitutions for a given language binding"""

    types: dict[str, str]  # syntax translations, C→this language
    syntax: Syntax  # how to describe a function in this language
    synopsis: str = ""  # content for the `.SH SYNOPSIS` man page section
    usage: str = ""  # content for the `.SH USAGE` man page section


LANGS = {
    "sharp": Lang(
        {
            "Agraph_t*": "SWIGTYPE_p_Agraph_t",
            "Agnode_t*": "SWIGTYPE_p_Agnode_t",
            "Agedge_t*": "SWIGTYPE_p_Agedge_t",
            "Agsym_t*": "SWIGTYPE_p_Agsym_t",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "SWIGTYPE_p_FILE",
            "bool": "bool",
            "int": "int",
            "void": None,
        },
        Syntax("gv.", "(", ", ", ");"),
    ),
    "go": Lang(
        {
            "Agraph_t* g": "graph_handle",
            "Agraph_t* sg": "subgraph_handle",
            "Agnode_t* n": "node_handle",
            "Agnode_t* t": "tail_node_handle",
            "Agnode_t* h": "head_node_handle",
            "Agedge_t* e": "edge_handle",
            "Agsym_t* a": "attr_handle",
            "char* gne": "type",
            "char* name": "name",
            "char* tname": "tail_name",
            "char* hname": "head_name",
            "char* attr": "attr_name",
            "char* val": "attr_value",
            "char* engine": "engine",
            "char* string": "string",
            "char** outdata": "outdata",
            "char* format": "format",
            "FILE* f": "channel",
            "void** data": "data_handle",
            "Agraph_t*": "graph_handle",
            "Agnode_t*": "node_handle",
            "Agedge_t*": "edge_handle",
            "Agsym_t*": "attribute_handle",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "channel",
            "bool": "bool",
            "int": "int",
            "void**": "data_handle",
            "void": None,
        },
        Syntax("gv.", "(", ", ", ");"),
    ),
    "guile": Lang(
        {
            "Agraph_t* g": "graph_handle",
            "Agraph_t* sg": "subgraph_handle",
            "Agnode_t* n": "node_handle",
            "Agnode_t* t": "tail_node_handle",
            "Agnode_t* h": "head_node_handle",
            "Agedge_t* e": "edge_handle",
            "Agsym_t* a": "attr_handle",
            "char* gne": "type",
            "char* name": "name",
            "char* tname": "tail_name",
            "char* hname": "head_name",
            "char* attr": "attr_name",
            "char* val": "attr_value",
            "char* engine": "engine",
            "char* string": "string",
            "char** outdata": "outdata",
            "char* format": "format",
            "FILE* f": "channel",
            "void** data": "data_handle",
            "Agraph_t*": "graph_handle",
            "Agnode_t*": "node_handle",
            "Agedge_t*": "edge_handle",
            "Agsym_t*": "attribute_handle",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "channel",
            "bool": "bool",
            "int": "int",
            "void**": "data_handle",
            "void": None,
        },
        Syntax("gv.", "(", ", ", ");"),
        '(load-extension "./libgv.so" "SWIG_init")',
    ),
    "java": Lang(
        {
            "Agraph_t*": "SWIGTYPE_p_Agraph_t",
            "Agnode_t*": "SWIGTYPE_p_Agnode_t",
            "Agedge_t*": "SWIGTYPE_p_Agedge_t",
            "Agsym_t*": "SWIGTYPE_p_Agsym_t",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "SWIGTYPE_p_FILE",
            "bool": "bool",
            "int": "int",
            "void": None,
        },
        Syntax("gv.", "(", ", ", ");"),
        'System.loadLibrary("gv");',
    ),
    "lua": Lang(
        {
            "Agraph_t* g": "graph_handle",
            "Agraph_t* sg": "subgraph_handle",
            "Agnode_t* n": "node_handle",
            "Agnode_t* t": "tail_node_handle",
            "Agnode_t* h": "head_node_handle",
            "Agedge_t* e": "edge_handle",
            "Agsym_t* a": "attr_handle",
            "char* gne": "type",
            "char* name": "name",
            "char* tname": "tail_name",
            "char* hname": "head_name",
            "char* attr": "attr_name",
            "char* val": "attr_value",
            "char* engine": "engine",
            "char* string": "string",
            "char** outdata": "outdata",
            "char* format": "format",
            "FILE* f": "channel",
            "void** data": "data_handle",
            "Agraph_t*": "graph_handle",
            "Agnode_t*": "node_handle",
            "Agedge_t*": "edge_handle",
            "Agsym_t*": "attribute_handle",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "channel",
            "bool": "bool",
            "int": "int",
            "void**": "data_handle",
            "void": None,
        },
        Syntax("gv.", "(", ", ", ");"),
        """\
#!/usr/bin/lua
require('gv')""",
    ),
    "perl": Lang(
        {
            "Agraph_t* g": "graph_handle",
            "Agraph_t* sg": "subgraph_handle",
            "Agnode_t* n": "node_handle",
            "Agnode_t* t": "tail_node_handle",
            "Agnode_t* h": "head_node_handle",
            "Agedge_t* e": "edge_handle",
            "Agsym_t* a": "attr_handle",
            "char* gne": "type",
            "char* name": "name",
            "char* tname": "tail_name",
            "char* hname": "head_name",
            "char* attr": "attr_name",
            "char* val": "attr_value",
            "char* engine": "engine",
            "char* string": "string",
            "char** outdata": "outdata",
            "char* format": "format",
            "FILE* f": "channel",
            "void** data": "data_handle",
            "Agraph_t*": "graph_handle",
            "Agnode_t*": "node_handle",
            "Agedge_t*": "edge_handle",
            "Agsym_t*": "attribute_handle",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "channel",
            "bool": "bool",
            "int": "int",
            "void**": "data_handle",
            "void": None,
        },
        Syntax("gv::", "(", ", ", ");"),
        """\
#!/usr/bin/perl
use gv;""",
    ),
    "php": Lang(
        {
            "Agraph_t* g": "graph_handle",
            "Agraph_t* sg": "subgraph_handle",
            "Agnode_t* n": "node_handle",
            "Agnode_t* t": "tail_node_handle",
            "Agnode_t* h": "head_node_handle",
            "Agedge_t* e": "edge_handle",
            "Agsym_t* a": "attr_handle",
            "char* gne": "type",
            "char* name": "name",
            "char* tname": "tail_name",
            "char* hname": "head_name",
            "char* attr": "attr_name",
            "char* val": "attr_value",
            "char* engine": "engine",
            "char* string": "string",
            "char** outdata": "outdata",
            "char* format": "format",
            "FILE* f": "channel",
            "void** data": "data_handle",
            "Agraph_t*": "graph_handle",
            "Agnode_t*": "node_handle",
            "Agedge_t*": "edge_handle",
            "Agsym_t*": "attribute_handle",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "channel",
            "bool": "bool",
            "int": "int",
            "void**": "data_handle",
            "void": None,
        },
        Syntax("gv::", "(", ", ", ");"),
        """\
#!/usr/bin/php
<?
include("gv.php")
...
?>""",
    ),
    "python": Lang(
        {
            "Agraph_t* g": "graph_handle",
            "Agraph_t* sg": "subgraph_handle",
            "Agnode_t* n": "node_handle",
            "Agnode_t* t": "tail_node_handle",
            "Agnode_t* h": "head_node_handle",
            "Agedge_t* e": "edge_handle",
            "Agsym_t* a": "attr_handle",
            "char* gne": "type",
            "char* name": "name",
            "char* tname": "tail_name",
            "char* hname": "head_name",
            "char* attr": "attr_name",
            "char* val": "attr_value",
            "char* engine": "engine",
            "char* string": "string",
            "char** outdata": "outdata",
            "char* format": "format",
            "FILE* f": "channel",
            "void** data": "data_handle",
            "Agraph_t*": "graph_handle",
            "Agnode_t*": "node_handle",
            "Agedge_t*": "edge_handle",
            "Agsym_t*": "attribute_handle",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "channel",
            "bool": "bool",
            "int": "int",
            "void**": "data_handle",
            "void": None,
        },
        Syntax("gv.", "(", ", ", ");"),
        """\
#!/usr/bin/python
import sys
import gv""",
    ),
    "R": Lang(
        {
            "Agraph_t*": "SWIGTYPE_p_Agraph_t",
            "Agnode_t*": "SWIGTYPE_p_Agnode_t",
            "Agedge_t*": "SWIGTYPE_p_Agedge_t",
            "Agsym_t*": "SWIGTYPE_p_Agsym_t",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "SWIGTYPE_p_FILE",
            "bool": "bool",
            "int": "int",
            "void": None,
        },
        Syntax("gv.", "(", ", ", ");"),
        'System.loadLibrary("gv");',
    ),
    "ruby": Lang(
        {
            "Agraph_t* g": "graph_handle",
            "Agraph_t* sg": "subgraph_handle",
            "Agnode_t* n": "node_handle",
            "Agnode_t* t": "tail_node_handle",
            "Agnode_t* h": "head_node_handle",
            "Agedge_t* e": "edge_handle",
            "Agsym_t* a": "attr_handle",
            "char* gne": "type",
            "char* name": "name",
            "char* tname": "tail_name",
            "char* hname": "head_name",
            "char* attr": "attr_name",
            "char* val": "attr_value",
            "char* engine": "engine",
            "char* string": "string",
            "char** outdata": "outdata",
            "char* format": "format",
            "FILE* f": "channel",
            "void** data": "data_handle",
            "Agraph_t*": "graph_handle",
            "Agnode_t*": "node_handle",
            "Agedge_t*": "edge_handle",
            "Agsym_t*": "attribute_handle",
            "char*": "string",
            "const char*": "string",
            "char**": "outdata",
            "FILE*": "channel",
            "bool": "bool",
            "int": "int",
            "void**": "data_handle",
            "void": None,
        },
        Syntax("Gv.", "(", ", ", ");"),
        """\
#!/usr/bin/ruby
require 'gv'""",
    ),
    "tcl": Lang(
        {
            "Agraph_t* g": "<graph_handle>",
            "Agraph_t* sg": "<subgraph_handle>",
            "Agnode_t* n": "<node_handle>",
            "Agnode_t* t": "<tail_node_handle>",
            "Agnode_t* h": "<head_node_handle>",
            "Agedge_t* e": "<edge_handle>",
            "Agsym_t* a": "<attr_handle>",
            "char* gne": "<type>",
            "char* name": "<name>",
            "char* tname": "<tail_name>",
            "char* hname": "<head_name>",
            "char* attr": "<attr_name>",
            "char* val": "<attr_value>",
            "char* engine": "<engine>",
            "char* string": "<string>",
            "char** outdata": "<outdata>",
            "char* format": "<format>",
            "FILE* f": "<channel>",
            "void** data": "<data_handle>",
            "Agraph_t*": "<graph_handle>",
            "Agnode_t*": "<node_handle>",
            "Agedge_t*": "<edge_handle>",
            "Agsym_t*": "<attr_handle>",
            "char*": "<string>",
            "const char*": "<string>",
            "char**": "<outdata>",
            "FILE*": "<channel>",
            "bool": "<boolean_string>",
            "int": "<integer_string>",
            "void**": "<data_handle>",
            "void": None,
        },
        Syntax("gv::", "", " ", ""),
        """\
#!/usr/bin/tclsh
package require gv""",
        "Requires tcl8.3 or later.",
    ),
}

# man page content template
TEMPLATE = """\
.TH gv 3{lang_lower} "{buildtime}"

.SH NAME

gv_{lang} - graph manipulation in {lang}

.SH SYNOPSIS

{synopsis}

.SH USAGE

{usage}

.SH INTRODUCTION

.B gv_{lang}
is a dynamically loaded extension for
.B {lang}
that provides access to the graph facilities of
.B graphviz.

.SH COMMANDS

{commands}

.SH KEYWORDS

graph, dot, neato, fdp, circo, twopi, {lang}.

"""


def gv_doc_synopsis(lang: Lang) -> str:
    """generate a substitution for the `.SH SYNOPSIS` section"""
    return "\n.br\n".join(lang.synopsis.split("\n"))


def gv_doc_usage(lang: Lang) -> str:
    """generate a substitution for the `.SH USAGE` section"""
    return "\n.P\n".join(lang.usage.split("\n"))


def gv_doc_commands(lang: Lang) -> str:
    """generate a substitution for the `.SH COMMANDS` section"""

    res = []
    fn = Path(__file__).parent / "gv.i"
    with open(fn, "rt", encoding="utf-8") as f:
        t = f.read()

        t = re.sub(r"(.|\n)*?%\{", "", t, count=1, flags=re.MULTILINE)
        t = re.sub(r"%\}(.|\n)*", "", t, count=1, flags=re.MULTILINE)
        t = re.sub(r"\bextern\b", "", t)

        for rec in t.split("\n"):

            rec = rec.strip(" \t;)")
            if len(rec) == 0:
                continue

            if c := re.match(r"/\*\*\* (?P<docstring>.*) \*/$", rec):
                res += [".TP", c.group("docstring"), ".br"]
                continue

            if c := re.match(r"/\*\* (?P<docstring>.*) \*/$", rec):
                res += [".TP", f"\\fB{c.group('docstring')}\\fR", ".br"]
                continue

            if re.match(r"/\*.*\*/$", rec):
                continue

            if re.match("//", rec):
                continue

            if re.match("#", rec):
                continue

            params: list[tuple[str, str]] = []
            func = None
            functype = None
            for i, type_name in enumerate(re.split(r"[\(,]", rec)):
                type_name = type_name.strip()
                type_name = re.sub(r"[ 	]+(\**)", r"\1 ", type_name)
                type, name = type_name.rsplit(maxsplit=1)
                if i == 0:
                    func = name
                    functype = type
                else:
                    params += [(type, name)]

            par = []
            for paramtype, param in params:

                if translation := lang.types.get(f"{paramtype} {param}"):
                    par += [translation]
                else:
                    par += [f"{lang.types[paramtype]} {param}"]

            rtype = ""
            if function_type := lang.types[functype]:
                rtype = f"\\fI{function_type}\\fR "

            res += [
                f"{rtype}\\fB{lang.syntax.nameprefix}{func}\\fR "
                f"\\fI{lang.syntax.paramstart}"
                f"{lang.syntax.paramsep.join(par)}{lang.syntax.paramend}\\fR",
                ".br",
            ]

    return "\n".join(res)


def main():
    """entry point"""

    buildtime = os.environ.get("SOURCE_DATE_EPOCH")
    if buildtime is None:
        buildtime = time.strftime("%d %B %Y", time.gmtime())

    for name, lang in LANGS.items():
        with open(f"gv.3{name.lower()}", "wt", encoding="utf-8") as f:
            f.write(
                TEMPLATE.format(
                    lang_lower=name.lower(),
                    buildtime=buildtime,
                    lang=name,
                    synopsis=gv_doc_synopsis(lang),
                    usage=gv_doc_usage(lang),
                    commands=gv_doc_commands(lang),
                )
            )


if __name__ == "__main__":
    main()
