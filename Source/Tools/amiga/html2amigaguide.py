#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
html2amigaguide.py - Dive Into Python HTML -> AmigaGuide (AmigaPython).

Host-side Python 3 tool. Emits V40-friendly AmigaGuide with:
  @DATABASE, @$VER, @(C), @AUTHOR, @MASTER, @SMARTWRAP, @HELP, @INDEX
  @NODE / @ENDNODE, @NEXT / @PREV / @TOC / @TITLE
  @{b} @{i} @{u} @{code} @{body} @{plain} text attributes
  AmigaPython note sidebars where desktop content differs

Usage:
  python3 html2amigaguide.py diveintopython.html OUTDIR
"""

from __future__ import print_function

import html as html_lib
import re
import sys
from html.parser import HTMLParser
from pathlib import Path

CHAPTERS = [
    ("install", "1. Installing Python", "DIP01.guide"),
    ("odbchelper", "2. Your First Python Program", "DIP02.guide"),
    ("datatypes", "3. Native Datatypes", "DIP03.guide"),
    ("apihelper", "4. The Power Of Introspection", "DIP04.guide"),
    ("fileinfo", "5. Objects and Object-Orientation", "DIP05.guide"),
    ("filehandling", "6. Exceptions and File Handling", "DIP06.guide"),
    ("re", "7. Regular Expressions", "DIP07.guide"),
    ("dialect", "8. HTML Processing", "DIP08.guide"),
    ("kgp", "9. XML Processing", "DIP09.guide"),
    ("streams", "10. Scripts and Streams", "DIP10.guide"),
    ("oa", "11. HTTP Web Services", "DIP11.guide"),
    ("soap", "12. SOAP Web Services", "DIP12.guide"),
    ("roman", "13. Unit Testing", "DIP13.guide"),
    ("roman1.5", "14. Test-First Programming", "DIP14.guide"),
    ("roman2", "15. Refactoring", "DIP15.guide"),
    ("regression", "16. Functional Programming", "DIP16.guide"),
    ("plural", "17. Dynamic functions", "DIP17.guide"),
    ("soundex", "18. Performance Tuning", "DIP18.guide"),
]

APPENDICES = [
    ("furtherreading", "A. Further reading", "DIPA.guide"),
    ("abstracts", "B. A 5-minute review", "DIPB.guide"),
    ("tips", "C. Tips and tricks", "DIPC.guide"),
    ("examples", "D. List of examples", "DIPD.guide"),
    ("revhistory", "E. Revision history", "DIPE.guide"),
    ("about", "F. About the book", "DIPF.guide"),
    ("gfdl", "G. GNU Free Documentation License", "DIPG.guide"),
    ("license", "H. Python license", "DIPH.guide"),
]

WIDTH = 76

CHAPTER_NOTES = {
    "install": (
        "Skip the Windows / Mac / Linux install procedures in this "
        "chapter. AmigaPython is a copy-anywhere drawer: keep lib/ "
        "beside the Python binary. Optional ASSIGN Python: is only "
        "convenience. See AmigaPython.guide and the release README."
    ),
    "odbchelper": (
        "Language examples are fine on AmigaPython 2.7. Run scripts "
        "with Python script.py (Shell) or set Default Tool on a "
        "project icon. Interactive: double-click Python for CON:."
    ),
    "datatypes": (
        "Core datatypes match CPython 2.7. Prefer Amiga paths "
        "(DH0:Drawer/file); os.path understands them."
    ),
    "apihelper": (
        "Introspection APIs work. help() / pydoc need a console; from "
        "Workbench use the Python CON: window."
    ),
    "fileinfo": (
        "OOP examples are portable. Opens go through AmigaDOS/PosixLib; "
        "see AmigaPython.guide Workbench notes about project locks."
    ),
    "filehandling": (
        "Exceptions are standard. Paths use volume:drawer/file, not "
        "Unix /home/... Parent navigation differs from Unix '../'."
    ),
    "re": (
        "Module re is available. The old regex/regsub modules are gone "
        "(as in upstream 2.5+)."
    ),
    "dialect": (
        "HTML examples need local files or a download. Networking uses "
        "bsdsocket.library (e.g. Roadshow). I-Net225 is unsupported."
    ),
    "kgp": (
        "xml.sax / ElementTree / Expat are in this build. Large XML "
        "needs RAM and stack; raise stack if needed."
    ),
    "streams": (
        "Shell stdin/stdout are fine. Workbench uses CON:. os.popen / "
        "PIPE: may need a fixed l:queue-handler (AmigaPython.guide)."
    ),
    "oa": (
        "HTTP needs bsdsocket.library first. No SSL in classic "
        "AmigaPython; HTTPS examples will fail."
    ),
    "soap": (
        "SOAP / network demos need a TCP stack. Many examples assume "
        "desktop Internet tools; treat as reading unless adapted."
    ),
    "roman": (
        "unittest works. Suites are slow on 68020-class machines; see "
        "also RunTest.py in the release drawer."
    ),
    "roman1.5": "Test-first ideas apply; expect long runtimes on classic Amiga.",
    "roman2": "Refactoring discussion is language-level and portable.",
    "regression": (
        "Functional examples are fine. Generators and itertools exist "
        "in 2.7; watch memory on large sequences."
    ),
    "plural": "Dynamic function examples are portable Python 2.7.",
    "soundex": (
        "Book timings are for desktop PCs. On classic Amiga prefer "
        "clarity; raise stack for recursion-heavy code."
    ),
    "furtherreading": (
        "Many links assume desktop OS docs. For Amiga modules see "
        "Help/Amiga/ and AmigaPython.guide."
    ),
    "abstracts": (
        "Review material is language-level; ignore install-platform "
        "bits from chapter 1."
    ),
    "tips": (
        "Some tips mention Unix shells, Windows IDEs, or Mac tools. On "
        "Amiga use Shell and AmigaPython.guide for port specifics."
    ),
    "examples": (
        "Example list refers to the book's sample tree, not shipped "
        "here. Local demos: Demo/ in the release drawer."
    ),
    "revhistory": (
        "Book history only; AmigaPython port history is in Help/CHANGES."
    ),
    "about": (
        "About the original book. This AmigaGuide is a modified edition "
        "for AmigaPython (amigazen project)."
    ),
    "gfdl": "License text for the book. Keep with any redistribution.",
    "license": (
        "Python license excerpt from the book. Also see "
        "Help/DISCL_and_COPYRIGHT and Help/LICENSE."
    ),
}

# Match cleaned h2 titles (after &nbsp; -> space).
SECTION_NOTES = [
    (
        re.compile(r"^1\.2\.\s*Python on Windows", re.I),
        "Not used on Amiga. Skip to The Interactive Shell, or use "
        "AmigaPython.guide instead of this chapter.",
    ),
    (
        re.compile(r"^1\.3\.\s*Python on Mac OS X", re.I),
        "Not used on Amiga. Skip Mac install sections.",
    ),
    (
        re.compile(r"^1\.4\.\s*Python on Mac OS 9", re.I),
        "Not used on Amiga. Skip Mac OS 9 install sections.",
    ),
    (
        re.compile(r"^1\.5\.\s*Python on RedHat", re.I),
        "Not used on Amiga. Skip Linux RPM sections.",
    ),
    (
        re.compile(r"^1\.6\.\s*Python on Debian", re.I),
        "Not used on Amiga. Skip Debian apt sections.",
    ),
    (
        re.compile(r"^1\.7\.\s*Python Installation from Source", re.I),
        "Building AmigaPython is a developer task (VBCC / ToolKit). "
        "End users: use the prebuilt release drawer.",
    ),
    (
        re.compile(r"^1\.8\.\s*The Interactive Shell", re.I),
        "On Amiga: Shell -> Python, or double-click Python for CON:. "
        "Exit with Ctrl+C / close gadget.",
    ),
]

ENTITY_MAP = {
    "nbsp": " ",
    "lt": "<",
    "gt": ">",
    "amp": "&",
    "quot": '"',
    "apos": "'",
    "ldquo": '"',
    "rdquo": '"',
    "lsquo": "'",
    "rsquo": "'",
    "mdash": "--",
    "ndash": "-",
    "hellip": "...",
    "copy": "(C)",
    "reg": "(R)",
    "trade": "(TM)",
    "bull": "*",
    "middot": "*",
    "times": "x",
    "divide": "/",
    "minus": "-",
}


def decode_entity(name_or_num):
    if name_or_num.startswith("#x") or name_or_num.startswith("#X"):
        try:
            return chr(int(name_or_num[2:], 16))
        except ValueError:
            return ""
    if name_or_num.startswith("#"):
        try:
            return chr(int(name_or_num[1:]))
        except ValueError:
            return ""
    if name_or_num in ENTITY_MAP:
        return ENTITY_MAP[name_or_num]
    try:
        return html_lib.unescape("&%s;" % name_or_num)
    except Exception:
        return ""


def to_latin1(s):
    """Map Unicode leftovers to Latin-1 / ASCII for Amiga text."""
    repl = {
        "\u2018": "'",
        "\u2019": "'",
        "\u201c": '"',
        "\u201d": '"',
        "\u2013": "-",
        "\u2014": "--",
        "\u2026": "...",
        "\u00a0": " ",
        "\u2192": "->",
        "\u2190": "<-",
    }
    for a, b in repl.items():
        s = s.replace(a, b)
    return s.encode("latin-1", errors="replace").decode("latin-1")


def ag_escape_text(s):
    """Escape @ for body text (not inside commands we emit ourselves)."""
    return s.replace("\\", "\\\\").replace("@", "\\@")


def amiga_note_block(note):
    """Sidebar using AmigaGuide attributes (SMARTWRAP-friendly)."""
    note = to_latin1(note.strip())
    lines = [
        "@{b}AmigaPython note@{ub}",
        "",
        "@{i}%s@{ui}" % ag_escape_text(note),
        "",
    ]
    return "\n".join(lines)


class GuideBuilder(HTMLParser):
    """Convert an HTML fragment into AmigaGuide body text + section list.

    sections: list of (node_name, title, body_text)
    First section may be intro (before first h2).
    """

    SKIP_TAGS = set(
        [
            "script",
            "style",
            "head",
            "meta",
            "link",
            "img",
            "hr",
            "col",
            "colgroup",
        ]
    )

    def __init__(self):
        HTMLParser.__init__(self, convert_charrefs=False)
        self.sections = []
        self._section_title = "Intro"
        self._node_i = 0
        self._buf = []
        self._skip = 0
        self._in_pre = 0
        self._in_code = 0
        self._heading_level = 0
        self._heading_bits = []
        self._pending_space = False
        self._at_para_start = True

    def _flush_section(self):
        text = self._finalize_buf()
        if not text.strip():
            self._buf = []
            return
        node = "S%d" % self._node_i
        self.sections.append((node, self._section_title, text))
        self._node_i += 1
        self._buf = []
        self._at_para_start = True

    def _finalize_buf(self):
        s = "".join(self._buf)
        s = s.replace("\r\n", "\n").replace("\r", "\n")
        # Drop any raw HTML that survived malformed markup.
        s = re.sub(r"(?is)<script[^>]*>.*?</script>", " ", s)
        s = re.sub(r"(?is)<style[^>]*>.*?</style>", " ", s)
        s = re.sub(r"<[^>]+>", "", s)
        s = re.sub(r"&nbsp;?", " ", s, flags=re.I)
        s = re.sub(r"&lt;?", "<", s, flags=re.I)
        s = re.sub(r"&gt;?", ">", s, flags=re.I)
        s = re.sub(r"&amp;?", "&", s, flags=re.I)
        s = re.sub(r"&#(\d+);?", lambda m: chr(int(m.group(1))) if int(m.group(1)) < 65536 else "", s)
        s = re.sub(r"&#x([0-9a-fA-F]+);?", lambda m: chr(int(m.group(1), 16)) if int(m.group(1), 16) < 65536 else "", s)
        s = re.sub(r"[ \t]+", " ", s)
        s = re.sub(r" *\n *", "\n", s)
        s = re.sub(r"\n{3,}", "\n\n", s)
        # Fix orphan attribute closers from bad nesting.
        s = re.sub(r"^@{ub}\s*", "", s)
        s = re.sub(r"\n@{ub}\s*\n", "\n", s)
        return to_latin1(s.strip())

    def _emit(self, s):
        if not s:
            return
        if self._pending_space and not self._at_para_start:
            if s[0] not in " \n\t.,;:!?)":
                self._buf.append(" ")
        self._pending_space = False
        self._buf.append(s)
        self._at_para_start = s.endswith("\n")

    def _emit_cmd(self, s):
        self._pending_space = False
        self._buf.append(s)
        self._at_para_start = s.endswith("\n")

    def handle_starttag(self, tag, attrs):
        tag = tag.lower()
        if tag in self.SKIP_TAGS:
            if tag in ("script", "style"):
                self._skip += 1
            return
        if self._skip:
            return
        if tag == "h2":
            if self._buf or self.sections:
                self._flush_section()
            self._heading_level = 2
            self._heading_bits = []
            self._emit_cmd("\n\n@{b}")
            return
        if tag == "h1":
            # Chapter title inside intro; not a section split.
            self._heading_level = 1
            self._heading_bits = []
            self._emit_cmd("\n\n@{b}")
            return
        if tag == "h3":
            self._heading_level = 3
            self._heading_bits = []
            self._emit_cmd("\n\n@{b}")
            return
        if tag in ("p", "div", "blockquote"):
            if not self._at_para_start:
                self._emit_cmd("\n\n")
            return
        if tag in ("br",):
            self._emit_cmd("@{line}\n")
            return
        if tag in ("li",):
            if not self._at_para_start:
                self._emit_cmd("\n")
            self._emit("* ")
            return
        if tag in ("ul", "ol", "dl", "table", "tr"):
            if not self._at_para_start:
                self._emit_cmd("\n\n")
            return
        if tag in ("td", "th"):
            self._emit("  ")
            return
        if tag in ("pre",):
            self._in_pre += 1
            self._emit_cmd("\n\n@{code}\n")
            return
        if tag in ("code", "tt", "kbd", "samp"):
            if self._in_pre:
                return
            self._in_code += 1
            self._emit_cmd("@{code}")
            return
        if tag in ("b", "strong"):
            self._emit_cmd("@{b}")
            return
        if tag in ("i", "em", "var", "dfn"):
            self._emit_cmd("@{i}")
            return
        if tag in ("u",):
            self._emit_cmd("@{u}")
            return
        if tag == "a":
            return
        if tag == "hr":
            self._emit_cmd("\n\n")
            return

    def handle_endtag(self, tag):
        tag = tag.lower()
        if tag in ("script", "style"):
            if self._skip:
                self._skip -= 1
            return
        if self._skip:
            return
        if tag in ("h1", "h2", "h3"):
            # Ignore closing tags for headings that started before this HTML slice.
            if not self._heading_level:
                return
            self._emit_cmd("@{ub}\n\n")
            title = re.sub(r"\s+", " ", "".join(self._heading_bits)).strip()
            if self._heading_level == 2 and title:
                self._section_title = title
            self._heading_level = 0
            self._heading_bits = []
            return
        if tag in ("p", "div", "blockquote", "li"):
            self._emit_cmd("\n\n")
            return
        if tag in ("ul", "ol", "dl", "table"):
            self._emit_cmd("\n")
            return
        if tag in ("pre",):
            if self._in_pre:
                self._in_pre -= 1
            self._emit_cmd("\n@{body}\n\n")
            return
        if tag in ("code", "tt", "kbd", "samp"):
            if self._in_pre:
                return
            if self._in_code:
                self._in_code -= 1
            self._emit_cmd("@{body}")
            self._pending_space = True
            return
        if tag in ("b", "strong"):
            self._emit_cmd("@{ub}")
            self._pending_space = True
            return
        if tag in ("i", "em", "var", "dfn"):
            self._emit_cmd("@{ui}")
            self._pending_space = True
            return
        if tag in ("u",):
            self._emit_cmd("@{uu}")
            self._pending_space = True
            return

    def handle_data(self, data):
        if self._skip:
            return
        if self._heading_level:
            self._heading_bits.append(data)
        if self._in_pre:
            self._emit_cmd(ag_escape_text(data.replace("\t", "    ")))
            return
        data = data.replace("\t", " ")
        data = re.sub(r"[ \t]*\n[ \t]*", " ", data)
        data = re.sub(r" +", " ", data)
        if not data:
            return
        self._emit(ag_escape_text(data))

    def handle_entityref(self, name):
        if self._skip:
            return
        ch = decode_entity(name)
        if ch:
            self.handle_data(ch)

    def handle_charref(self, name):
        if self._skip:
            return
        ch = decode_entity("#" + name)
        if ch:
            self.handle_data(ch)

    def close(self):
        HTMLParser.close(self)
        self._flush_section()


def find_anchor_pos(html, name):
    for pat in (
        'name="%s"' % name,
        "name='%s'" % name,
        'id="%s"' % name,
        "id='%s'" % name,
    ):
        p = html.find(pat)
        if p >= 0:
            return p
    return -1


def slice_after_anchor(html, pos):
    gt = html.find(">", pos)
    if gt < 0:
        return pos
    return gt + 1


def slice_sections(html, items):
    positions = []
    for name, title, fname in items:
        pos = find_anchor_pos(html, name)
        if pos < 0:
            raise SystemExit("anchor not found: %s" % name)
        positions.append((pos, name, title, fname))
    positions.sort(key=lambda t: t[0])
    out = []
    for i, (pos, name, title, fname) in enumerate(positions):
        start = slice_after_anchor(html, pos)
        end = positions[i + 1][0] if i + 1 < len(positions) else len(html)
        out.append((name, title, fname, html[start:end]))
    return out


def section_note_for_title(title):
    for cre, note in SECTION_NOTES:
        if cre.search(title):
            return note
    return None


def clean_title(title, fallback):
    title = re.sub(r"\s+", " ", title or "").strip()
    title = to_latin1(title)
    if not title:
        return fallback
    # AmigaGuide titles: keep reasonably short
    if len(title) > 60:
        title = title[:57] + "..."
    return title


def write_chapter_guide(path, chap_title, anchor, sections):
    """Write one chapter database with MAIN TOC + section nodes."""
    lines = []
    lines.append('@DATABASE "%s"' % path.name)
    lines.append("@$VER: %s 5.4-amiga (16.8.2026)" % path.name)
    lines.append(
        "@(C) Copyright 2000-2004 Mark Pilgrim. "
        "AmigaGuide edition for AmigaPython (amigazen project)."
    )
    lines.append(
        "@AUTHOR Mark Pilgrim; AmigaGuide conversion amigazen project"
    )
    lines.append("@MASTER http://diveintopython.org/")
    lines.append("@SMARTWRAP")
    lines.append("@WIDTH %d" % WIDTH)
    lines.append("@TAB 4")
    lines.append("@HELP DiveIntoPython.guide/AmigaNote")
    lines.append("@INDEX DiveIntoPython.guide/MAIN")
    lines.append(
        "@REM Modified AmigaGuide for AmigaPython 2.7.18. GFDL 1.1+."
    )
    lines.append("")

    # MAIN = chapter TOC
    lines.append('@NODE MAIN "%s"' % chap_title.replace('"', "'"))
    lines.append('@TITLE "%s"' % chap_title.replace('"', "'"))
    if sections:
        lines.append('@NEXT "%s"' % sections[0][0])
    lines.append('@TOC "MAIN"')
    lines.append("")
    lines.append("@{b}%s@{ub}" % ag_escape_text(chap_title))
    lines.append("")
    note = CHAPTER_NOTES.get(anchor)
    if note:
        lines.append(amiga_note_block(note).rstrip())
        lines.append("")
    lines.append("Sections:")
    lines.append("")
    for node, title, _body in sections:
        label = clean_title(title, node)
        lines.append('@{" %s " LINK "%s"}' % (label.replace('"', "'"), node))
    lines.append("")
    lines.append(
        '@{" Contents (book) " LINK "DiveIntoPython.guide/MAIN"}'
    )
    lines.append(
        '@{" AmigaPython.guide " LINK "AmigaPython.guide/MAIN"}'
    )
    lines.append("")
    lines.append("@ENDNODE")
    lines.append("")

    for i, (node, title, body) in enumerate(sections):
        title = clean_title(title, node)
        lines.append('@NODE %s "%s"' % (node, title.replace('"', "'")))
        lines.append('@TITLE "%s"' % title.replace('"', "'"))
        if i + 1 < len(sections):
            lines.append('@NEXT "%s"' % sections[i + 1][0])
        else:
            lines.append('@NEXT "MAIN"')
        if i == 0:
            lines.append('@PREV "MAIN"')
        else:
            lines.append('@PREV "%s"' % sections[i - 1][0])
        lines.append('@TOC "MAIN"')
        lines.append("@SMARTWRAP")
        lines.append("")
        snote = section_note_for_title(title)
        if snote:
            lines.append(amiga_note_block(snote).rstrip())
            lines.append("")
        lines.append(body)
        lines.append("")
        lines.append('@{" Chapter contents " LINK "MAIN"}')
        lines.append("")
        lines.append("@ENDNODE")
        lines.append("")

    path.write_bytes("\n".join(lines).encode("latin-1", errors="replace"))


def write_master(outdir, chapter_files, appendix_files):
    path = outdir / "DiveIntoPython.guide"
    lines = []
    lines.append('@DATABASE "DiveIntoPython.guide"')
    lines.append("@$VER: DiveIntoPython.guide 5.4-amiga (16.8.2026)")
    lines.append(
        "@(C) Copyright 2000-2004 Mark Pilgrim. "
        "AmigaGuide edition for AmigaPython (amigazen project)."
    )
    lines.append(
        "@AUTHOR Mark Pilgrim; AmigaGuide conversion amigazen project"
    )
    lines.append("@MASTER http://diveintopython.org/")
    lines.append("@SMARTWRAP")
    lines.append("@WIDTH %d" % WIDTH)
    lines.append("@TAB 4")
    lines.append("@HELP DiveIntoPython.guide/AmigaNote")
    lines.append("@INDEX DiveIntoPython.guide/MAIN")
    lines.append(
        "@REM Dive Into Python 5.4 AmigaGuide for AmigaPython 2.7.18. "
        "GFDL 1.1+; no Invariant Sections."
    )
    lines.append("")

    lines.append('@NODE MAIN "Dive Into Python"')
    lines.append('@TITLE "Dive Into Python"')
    lines.append('@NEXT "AmigaNote"')
    lines.append("")
    lines.append("@{b}Dive Into Python@{ub}")
    lines.append("")
    lines.append("By Mark Pilgrim (20 May 2004)")
    lines.append("")
    lines.append(
        "AmigaGuide conversion for AmigaPython 2.7.18 "
        "(amigazen project)."
    )
    lines.append("")
    lines.append('@{" Amiga note / license " LINK "AmigaNote"}')
    lines.append("")
    lines.append(
        "Boxed @{b}AmigaPython note@{ub} sidebars mark content that "
        "does not apply, or works differently, on classic Amiga."
    )
    lines.append("")
    lines.append("@{b}Chapters@{ub}")
    lines.append("")
    for title, fname in chapter_files:
        lines.append(
            '@{" %s " LINK "%s/MAIN"}' % (title[:50].replace('"', "'"), fname)
        )
    lines.append("")
    lines.append("@{b}Appendices@{ub}")
    lines.append("")
    for title, fname in appendix_files:
        lines.append(
            '@{" %s " LINK "%s/MAIN"}' % (title[:50].replace('"', "'"), fname)
        )
    lines.append("")
    lines.append(
        '@{" AmigaPython port guide " LINK "AmigaPython.guide/MAIN"}'
    )
    lines.append("")
    lines.append("@ENDNODE")
    lines.append("")

    lines.append('@NODE AmigaNote "Amiga note / license"')
    lines.append('@TITLE "Amiga note / license"')
    lines.append('@PREV "MAIN"')
    lines.append('@TOC "MAIN"')
    lines.append("")
    lines.append("@{b}Amiga note / license@{ub}")
    lines.append("")
    lines.append(
        "This is a modified AmigaGuide edition of Dive Into Python 5.4, "
        "converted for use with AmigaPython 2.7.18."
    )
    lines.append("")
    lines.append(
        "Copyright (C) 2000, 2001, 2002, 2003, 2004 Mark Pilgrim. "
        "Permission is granted to copy, distribute and/or modify this "
        "document under the terms of the GNU Free Documentation License, "
        "Version 1.1 or any later version published by the Free Software "
        "Foundation; with no Invariant Sections, no Front-Cover Texts, "
        "and no Back-Cover Texts. See DIPG.guide and COPYING.GFDL."
    )
    lines.append("")
    lines.append("@{b}How Amiga annotations work@{ub}")
    lines.append("")
    lines.append(
        "Each chapter opens with an AmigaPython note when needed. "
        "Extra notes appear on Windows / Mac / Linux install sections "
        "in chapter 1. Follow those notes instead of desktop install "
        "steps."
    )
    lines.append("")
    lines.append(
        '@{" AmigaPython.guide " LINK "AmigaPython.guide/MAIN"}'
    )
    lines.append("")
    lines.append("Original book: http://diveintopython.org/")
    lines.append("")
    lines.append('@{" Contents " LINK "MAIN"}')
    lines.append("")
    lines.append("@ENDNODE")
    lines.append("")

    path.write_bytes("\n".join(lines).encode("latin-1", errors="replace"))


def extract_gfdl_plain(html):
    pos = find_anchor_pos(html, "gfdl")
    end = find_anchor_pos(html, "license")
    if pos < 0:
        return ""
    if end < 0:
        end = len(html)
    # plain text only for COPYING.GFDL
    builder = GuideBuilder()
    try:
        builder.feed(html[slice_after_anchor(html, pos) : end])
        builder.close()
    except Exception:
        return ""
    parts = [body for _n, _t, body in builder.sections]
    text = "\n\n".join(parts)
    text = re.sub(r"@\{[^}]+\}", "", text)
    return text.strip()


def convert_chunk(html_chunk):
    builder = GuideBuilder()
    builder.feed(html_chunk)
    builder.close()
    out = []
    for node, title, body in builder.sections:
        title = clean_title(title, node)
        if not body.strip():
            continue
        # Merge accidental duplicate consecutive section titles.
        if out and out[-1][1] == title:
            prev_n, prev_t, prev_b = out[-1]
            out[-1] = (prev_n, prev_t, (prev_b + "\n\n" + body).strip())
            continue
        out.append((node, title, body))
    renum = []
    for i, (_n, title, body) in enumerate(out):
        renum.append(("S%d" % i, title, body))
    return renum


def main(argv):
    if len(argv) != 3:
        print("Usage: %s diveintopython.html OUTDIR" % argv[0], file=sys.stderr)
        return 2
    src = Path(argv[1])
    outdir = Path(argv[2])
    outdir.mkdir(parents=True, exist_ok=True)
    html = src.read_text(encoding="latin-1", errors="replace")

    all_items = CHAPTERS + APPENDICES
    chunks = slice_sections(html, all_items)
    nchap = len(CHAPTERS)
    chapter_files = []
    appendix_files = []

    for i, (anchor, title, fname, chunk) in enumerate(chunks):
        sections = convert_chunk(chunk)
        if not sections:
            sections = [("S0", title, "(No text extracted.)")]
        write_chapter_guide(outdir / fname, title, anchor, sections)
        if i < nchap:
            chapter_files.append((title, fname))
        else:
            appendix_files.append((title, fname))
        print(
            "wrote",
            fname,
            "(%d nodes, %d chars)"
            % (len(sections), sum(len(b) for _n, _t, b in sections)),
        )

    write_master(outdir, chapter_files, appendix_files)
    print("wrote DiveIntoPython.guide")

    gfdl = extract_gfdl_plain(html)
    (outdir / "COPYING.GFDL").write_bytes(
        (gfdl + "\n").encode("latin-1", errors="replace")
    )
    print("wrote COPYING.GFDL")

    readme = """Dive Into Python 5.4 (AmigaGuide edition)
========================================

Copyright (C) 2000, 2001, 2002, 2003, 2004 Mark Pilgrim.

Modified AmigaGuide edition for AmigaPython 2.7.18 (amigazen project).

GNU Free Documentation License 1.1+ (no Invariant Sections, no
Front-Cover / Back-Cover Texts). See COPYING.GFDL and DIPG.guide.

Open DiveIntoPython.guide in AmigaGuide / MultiView.

Uses AmigaGuide features: @DATABASE, @$VER, @(C), @AUTHOR, @MASTER,
@SMARTWRAP, @HELP, @INDEX, per-chapter MAIN contents, section nodes
with @NEXT/@PREV, @{b}/@{i}/@{code} attributes, and AmigaPython note
sidebars where desktop install/platform text does not apply.

  Help/AmigaPython.guide
  README (release drawer root)

Original: http://diveintopython.org/

Regenerate:
  python3 Source/Tools/amiga/html2amigaguide.py \\
    path/to/diveintopython.html Python/Help/DiveIntoPython
"""
    (outdir / "README").write_bytes(readme.encode("latin-1", errors="replace"))
    print("wrote README")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
