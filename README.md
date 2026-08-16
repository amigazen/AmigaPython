# Amiga Python 2

This is Amiga Python, a port of **Python 2.7.18** to classic AmigaOS — continuing Irmen de Jong’s AmigaPython line from the Aminet 2.0 releases.

## [amigazen project](http://www.amigazen.com)

*A web, suddenly*

*Forty years meditation*

*Minds awaken, free*

**amigazen project** is using modern software development tools and methods to update and rerelease classic Amiga open source software. Projects include a new AWeb, a new Amiga Python 2, and the ToolKit project — a universal SDK for Amiga.

Key to the amigazen project approach is ensuring every project can be built with the same common set of development tools and configurations, so the ToolKit project was created to provide a standard configuration for Amiga development. All *amigazen project* releases will be guaranteed to build against the ToolKit standard so that anyone can download and begin contributing straightaway without having to tailor the toolchain for their own setup.

The original authors of *Python* and of the classic Amiga port are not affiliated with amigazen project. This software is redistributed on terms described in the documentation, particularly the file LICENSE.md

The amigazen project philosophy is based on openness:

*Open* to anyone and everyone	- *Open* source and free for all	- *Open* your mind and create!

PRs for all projects are gratefully received at [GitHub](https://github.com/amigazen/). While the focus now is on classic 68k software, it is intended that all amigazen project releases can be ported to other Amiga-like systems including AROS and MorphOS where feasible.

## About Amiga Python

Python was first ported to and adapted for the Amiga in 1999–2000 by **Irmen de Jong**. His Aminet releases through Python 2.0 remain in this repository’s history and `archive/` folder.

This project continues that work by bringing Amiga Python to **Python 2.7.18** (the last Python 2 release, April 2020), built against the ToolKit SDK on a classic Amiga.

Amiga Python aims to be as complete a port as makes sense on AmigaOS 3.x: the usual 2.7 language and library surface, plus Amiga-native modules (`amiga`, `arexx`, ASL / catalog / icon helpers, and related site-python shims).

This project is **not** the Python that ships with AmigaOS 4. See the FAQ below. Collaboration on OS4 and other Amiga-like platforms is welcome in the amigazen project spirit of openness.

### Goals

- A **ToolKit-buildable** Python 2.7 for classic 68k Amiga (VBCC + PosixLib primary; SAS/C where still useful)
- Networking against modern **bsdsocket.library** (e.g. Roadshow); **I-Net225 dropped**
- Amiga integration that feels native: Dos / ASL / catalogs / icons / ARexx, documented in Library Reference–style reST
- Keep the port honest about classic Amiga limits (RAM, CPU, no full Unicode stack by default)

Official upstream docs: https://docs.python.org/2.7/

## What’s new in Python 2.x (2.0 → 2.7.18)

Relative to the classic AmigaPython **2.0** baseline, upstream Python 2 gained roughly a decade of language and library work. Highlights (see also `Python/Help/NEWS` and https://docs.python.org/2.7/whatsnew/):

### Language

| Area | Notes |
|------|--------|
| Scopes & generators | Nested scopes; `yield`; list / dict / set comprehensions |
| Booleans | `bool`, and `True` / `False` / `None` as constants |
| OO | New-style classes, descriptors, `property`, `super` |
| Syntax | Decorators and class decorators; `with` / context managers |
| Control flow | Unified `try` / `except` / `finally`; relative imports |
| Futures | `from __future__ import print_function` (and other futures) |
| 2.7 extras | Set literals; dict/set comprehensions; `str.format()`; `memoryview` |

### Standard library

| Area | Notes |
|------|--------|
| Data & CLI | `json`, `argparse`, `collections` (`OrderedDict`, `Counter`, …) |
| Crypto & process | `hashlib` / `hmac`, `subprocess` |
| Functional | `itertools`, `functools`, `contextlib`, `abc` |
| Apps | `logging`, stronger `unittest`, `email` overhaul |
| Net & XML | `urllib2` / `httplib` evolution; ElementTree + Expat |
| Packaging | Prefer current 2.7 module names in new code |

## What’s new in this Amiga port

Improvements beyond “stock 2.7 dropped on Amiga”:

### Build and platform

| Item | Notes |
|------|--------|
| Toolchain | **VBCC + PosixLib** / NDK 3.2 (ToolKit); SAS/C paths retained where useful |
| Networking | **bsdsocket.library** (Roadshow and friends); I-Net225 unsupported |
| I/O | Builtin **`_io`** so `io` / `tempfile` work (Amiga-friendly text path when Unicode is off) |
| Packaging | `zipimport` + `lib/python27.zip`-style stdlib; `_socket` as a LoadSeg plugin |
| Docs | Amiga modules in Library Reference–style reST (`Source/Amiga_Misc/Docs`, `Python/Help/Amiga`); end-user AmigaGuide in `Python/Help/` |

### Amiga modules and APIs

| Item | Notes |
|------|--------|
| `amiga` | Expanded POSIX-style surface (`access`, `sleep`/`usleep`, `waitpid`, `pathconf`/`sysconf`, `urandom`, `fsync`, `closerange`, …) |
| Dos helpers | Former Doslib / `_amigados` (`ArgParser`, `FIBF_*`, `touch`, Assign helpers) on `amiga` |
| Paths | `amiga.to_unix` / `amiga.from_unix` |
| Requesters | ASL `FileRequest` / `MessageBox`; locale `OpenCatalog`; icon `DiskObject` |
| site-python | OS4-style shims: `arexx`, `asl`, `catalog`, `icon`, `amigavars`, … |
| ARexx | `arexx` / `_arexx` high-level support retained and updated |
| Tests | AmigaTests coverage for Amiga extras and related builtins |

### Standalone release drawer

The shippable end-user product is the `Python/` drawer: copy it anywhere and run.
No installer script. Libraries live in `lib/` next to the binary. Optional
`ASSIGN Python:` is only a convenience. Docs include `Python.help`,
`Help/AmigaPython.guide`, and `Help/DiveIntoPython/` (GFDL AmigaGuide of
Dive Into Python). See `Python/README` and `BUILD.md`.

## About ToolKit

**ToolKit** exists because most Amiga software was written in the 1980s and 90s by individuals, each with their own layout for compilers, includes, and libs. Open source collaboration did not look like it does in 2025.

**ToolKit** from amigazen project is a work in progress toward a standardised Native Developer Kit plus compilers, build tools, and third-party components so projects can be built together without every contributor rewriting makefiles for their private toolchain.

All *amigazen project* releases ship in a ready-to-build configuration according to the ToolKit standard. Each ToolKit component is open source with its own GitHub repo; ToolKit itself will eventually be an easy install of redistributable pieces plus scripts for parts that cannot be freely redistributed.

## Roadmap

Classic AmigaPython stopped at **2.0** on Aminet. This line’s first job is a **stable, ToolKit-buildable Python 2.7.18** for AmigaOS 3.x with modern bsdsocket networking and usable Amiga modules.

Near-term focus:

- Harden the VBCC + PosixLib build and packaging (binary + stdlib zip + site-python + docs)
- Keep Amiga modules and docs aligned with what actually ships
- Expand AmigaTests; fix gaps that block real scripts (`_io`, socket plugin, path/ENV edge cases)
- Stay honest about RAM/CPU limits; prefer correctness over chasing every CPython optional extension

Longer term, where feasible: better optional modules, clearer Aminet-style distribution, and ports or shared work toward AROS / MorphOS / OS4 — without pretending classic 68k can host a full modern CPython 3.

## Other Python projects on Amiga

Several independent efforts bring Python (or a Python-like language) to Amiga-family systems. They solve different problems; none replaces the others.

| | **This project** (Amiga Python 2) | [python-amigaos4](https://github.com/geekychris/python-amigaos4) | [micropython-amiga-port](https://github.com/OoZe1911/micropython-amiga-port) |
|--|--|--|--|
| **What it is** | CPython **2.7.18** continued from Irmen’s AmigaPython | CPython **3.12.7** for AmigaOS 4 | **MicroPython** v1.28 (Python 3–like subset runtime) |
| **Platform** | Classic **68k** AmigaOS 3.x | **PowerPC** AmigaOS 4.1 (sam460ex / QEMU / hardware) | Classic **68020+** AmigaOS (and emulators) |
| **Lineage** | Aminet AmigaPython → ToolKit rebuild | New OS4 CPython port (not this repo) | Separate MicroPython port |
| **Build** | Native Amiga / ToolKit (**VBCC + PosixLib**; SAS/C where useful) | Cross-compile from Linux/macOS via **Docker** (GCC) | Cross-compile with **bebbo m68k-amigaos-gcc** |
| **Language model** | Python 2 (`str` as bytes; Unicode optional / often off) | Full Python 3 (Unicode-centric) | MicroPython dialect (f-strings, async, … within ROM feature level) |
| **Stdlib / packaging** | 2.7 library + zipimport; Amiga site-python | Broad 3.12 stdlib; **pip** for pure-Python wheels | Curated MicroPython modules + `.mpy` bytecode |
| **Networking** | bsdsocket; `_socket` as LoadSeg plugin | bsdsocket; SSL via optional AmiSSL | bsdsocket; HTTPS / SMTP via AmiSSL where enabled |
| **Amiga APIs** | `amiga`, ARexx, ASL, catalogs, icons, … | `_amiga` + `amiga.*` (Exec/DOS/Intuition/…) | `amiga.intuition`, `amiga.asl`, `arexx`, … |
| **Best when you want** | Classic Amiga scripting with full-ish CPython 2 and Aminet heritage | Modern Python 3 and PyPI on **OS4** | Small footprint, REPL, and Python 3–flavoured scripts on **68k** |

amigazen project’s Amiga Python 2 is deliberately the **classic CPython 2** line. For Python 3 on OS4, see geekychris’s port; for a lean 68k interpreter with a Python 3–like surface, see OoZe1911’s MicroPython port. Ideas and friendly cross-pollination are welcome; the codebases are not drop-in substitutes.

## Frequently Asked Questions

### Why Python 2 and not Python 3?

Because this project continues **Irmen de Jong’s AmigaPython**, which was a Python **2** port, and because classic Amiga is a constrained 68k / AmigaOS 3 environment.

Python 3 assumes a Unicode-centric runtime, a larger standard library, and a toolchain/runtime budget that does not map cleanly onto typical classic Amiga hardware and SDKs. Python **2.7.18** is the last 2.x release: it still matches the classic port’s model (`str` as bytes, optional Unicode), gives nearly a decade of language and library improvements over AmigaPython 2.0, and remains a realistic target for ToolKit builds.

If you need Python **3** on AmigaOS **4**, use [python-amigaos4](https://github.com/geekychris/python-amigaos4). If you want a smaller **Python 3–like** runtime on classic 68k, see [micropython-amiga-port](https://github.com/OoZe1911/micropython-amiga-port). Those are separate projects — see the comparison table above.

### Which key language features and modules *are* included?

This is **CPython 2.7.18** with a large Amiga-oriented builtin set. In normal builds you get, among other things:

**Language / runtime (2.7)**

- New-style classes, descriptors, `property`, `super`, decorators, `with` / context managers
- Generators (`yield`), nested scopes, list/dict/set comprehensions, `str.format()`
- Exceptions, `gc`, weakrefs, `ast` / `_ast`, warnings
- Primary text type is 8-bit `str` (see Unicode FAQ); `_io` is built so `io` / `tempfile` work without a full Unicode build

**Core / data builtins**

- `array`, `math`, `cmath`, `time`, `datetime`, `operator`, `strop`, `_struct` / `struct`
- `binascii`, `cStringIO`, `cPickle`, `marshal`, `_sre` / `re`, `_codecs` (subset — see below)
- `_collections`, `itertools`, `_functools`, `_random` / `random`, `_bisect`, `_heapq`, `_csv`
- `md5`, `sha`, `_hashlib` (with Amiga `crc.library` support where wired), `zlib`, `zipimport`
- `pyexpat` / Expat XML; `select`, `errno`, `imp`, `_symtable`, `_weakref`, `_warnings`

**OS / Amiga**

- Builtin `amiga` (POSIX-style OS API used via `os`), plus Dos / ASL / catalog / icon helpers
- `environment`, `_arexx` / `arexx`, `amigagui`, `amigalibs`
- `Lib/site-python` shims (`asl`, `catalog`, `icon`, `amigavars`, legacy `dos`, …)
- Pure-Python 2.7 stdlib on disk / zip where packaged (`os`, `os.path`, `json`, `argparse`, `email`, … — subject to missing C backends)

**Networking (optional at runtime)**

- `_socket` as a **LoadSeg plugin** (not a startup builtin): needs `bsdsocket.library`; interpreter starts without opening the stack
- Builtins `pwd` / `grp` use PosixLib stubs without usergroup; `crypt` / `syslog` open `usergroup.library` / require a live `SocketBase` and raise `SystemError` if missing (no hard crash through a dummy library base)

### Which key language features and modules are *not* included (or not default)?

These are the important gaps relative to a “full” desktop CPython 2.7 — mostly by design for classic Amiga size and ToolKit scope:

**Language / runtime**

- **Full Unicode** off by default (`Py_USING_UNICODE` undefined): no wide `unicode` type as the primary text model; `unicodedata` is not built in
- **No threads** (`WITH_THREAD` undefined): no `thread` / `threading` as on Unix CPython
- **Incomplete codecs** when Unicode is off — see the dedicated FAQ below

**OS builtin naming (`amiga` vs `posix`)**

- On Unix, `import os` re-exports the builtin named **`posix`**.
- On this port, **`posixmodule.c` is not compiled** (VBCC or SAS/C Amiga builds). The OS builtin is **`amiga`**; `os.py` does `from amiga import *`.
- After `amiga` is initialized, **`sys.modules['posix']` is set to the same module** so naive `import posix` still works. Portable code should keep using **`import os`**. Use `import amiga` for Amiga-only APIs (ASL, catalogs, Dos helpers, …).
- Do not add `posix` to `config.c`’s inittab: `os.py` checks for builtin name `posix` before `amiga`, and would take the wrong bootstrap path.

**C extensions typically absent or not shipping as builtins**

- `_ssl` / OpenSSL-style TLS (no AmiSSL-backed `_ssl` in this classic line yet)
- `_ctypes`, `_sqlite3`, `_multiprocessing`, `_tkinter`, `_bsddb`, `_hotshot` / `_lsprof` profilers
- `mmap`, `bz2`, `audioop`, `parser` (pgen), and many other optional Unix modules

**Platform / packaging**

- Not a drop-in for scripts that assume Unix processes, fork, full POSIX signals, or Windows APIs
- **I-Net225** networking is unsupported; use modern **bsdsocket** (Roadshow, UAE net, …)
- `pip` / binary wheels are not a supported distribution story on classic 68k the way they are on OS4 CPython 3

If a pure-Python stdlib module imports a missing C accelerator, that feature fails even though the `.py` file may be on the tree. Prefer AmigaTests and the Amiga docs under `Python/Help/Amiga/` for what is actually exercised.

### Why is the codec / `str.encode` story incomplete?

The codec machinery is only half there in the default Amiga build, because Unicode is off.

**How CPython 2 normally does it**

Calls such as `"abc".encode("ascii")` or `u"café".encode("utf-8")`, and much of XML / ElementTree I/O, go through:

1. the pure-Python `encodings` package, which
2. calls into the builtin **`_codecs`** module for the real work (`ascii_encode`, `utf_8_encode`, `latin_1_encode`, …).

Those `_codecs.*_encode` / `*_decode` helpers are implemented with the Unicode C API (`PyUnicode_FromObject`, `PyUnicode_EncodeASCII`, …). In stock `_codecsmodule.c` they sit behind `#ifdef Py_USING_UNICODE`.

**What this port does**

Default builds have `#undef Py_USING_UNICODE`. At compile time those helpers are **not** entered into `_codecs`’s method table. You still get a `_codecs` module (registry / lookup stubs), but **not** the usual C encoders.

So `"abc".encode("ascii")` fails when the encodings layer looks for `_codecs.ascii_encode` (AmigaTests skip with *“ascii_encode not in _codecs yet”*). **ElementTree** often hits the same wall even though **`pyexpat`** works for lower-level XML.

**What still works**

- Plain 8-bit `str` operations that never ask for an encoding
- `_io` text paths shimmed for no-Unicode (identity / latin-1-ish; skip full codec lookup)
- `pyexpat` when you stay on the Expat API and avoid ElementTree’s encode path

**“Until more `_codecs` encoders are wired”** means either turning Unicode on (expensive on classic Amiga), or adding 8-bit-only C stubs for common encodings (`ascii`, `latin-1`, maybe UTF-8 as bytes↔bytes) that do not need `PyUnicode_*`. Until then: registry present, real encode/decode C builtins largely absent.

### Why doesn’t it support Unicode (by default)?

Full CPython Unicode (`Py_USING_UNICODE`, wide builds, `unicodedata`, and the text stack that assumes them) is expensive in code size, RAM, and complexity on classic Amiga. This port often builds **without** `Py_USING_UNICODE` so the interpreter and `_io` stay usable; text is primarily **8-bit `str`** (Latin-1 / locale-ish), which matches how most classic Amiga software already treats strings.

That is a deliberate trade-off, not an unfinished checkbox. Work continues on Amiga-friendly I/O when Unicode is off. Enabling a fuller Unicode build later remains possible where memory and toolchain allow — it is not the default “it just works on a 68020 with modest Fast RAM” configuration.

MicroPython on Amiga and CPython 3 on OS4 take different Unicode choices because they target different runtimes and hardware budgets.

### Is this the same as OS4 Python / python-amigaos4?

**No.** This repository is the **classic Amiga / AmigaPython** line updated to 2.7.18 under amigazen project.

- Stock or third-party **AmigaOS 4** Python installs, and [geekychris/python-amigaos4](https://github.com/geekychris/python-amigaos4) (CPython 3.12 on PPC OS4), are separate lineages, ABIs, and packaging.
- Some site-python names here are shaped for familiar OS4-style usage (`arexx`, `asl`, `catalog`, `icon`, …) where that helps Amiga programmers, but this does not replace OS4 Python and is not affiliated with Hyperion or with python-amigaos4.

See **Other Python projects on Amiga** above for a side-by-side comparison.

### How does this relate to MicroPython on Amiga?

[micropython-amiga-port](https://github.com/OoZe1911/micropython-amiga-port) is a **MicroPython** port to classic 68k — a different interpreter (not CPython), with a smaller footprint and a Python 3–flavoured language surface. It is excellent when you want a lean REPL and curated modules on an A1200-class machine.

This project is **CPython 2.7**: larger, closer to “desktop” Python 2 scripts and libraries from the AmigaPython era, with ToolKit-native builds. Use MicroPython when size and a modern dialect matter more than CPython 2 compatibility; use Amiga Python 2 when you want the Aminet/CPython 2 line.

### How does this relate to Aminet AmigaPython 2.0?

Irmen’s Aminet packages (through Python 2.0) are the historical baseline. Sources and notes from that era live in git history and `archive/`. This tree is the continuation: same Amiga-first spirit, **2.7.18** language/library, ToolKit-oriented build, Roadshow-era networking instead of AmiTCP/I-Net225 assumptions.

Historic package notes: [Aminet Python20](https://www.aminet.net/package/dev/lang/Python20). Contact details in that era readme are obsolete — use the contacts below.

### What AmigaOS versions does it target?

Built and tested with **NDK 3.2** / ToolKit expectations for AmigaOS 3.x. Networking expects a **bsdsocket.library** stack (Roadshow, UAE net, etc.). Older Workbench releases may work with progressive runtime checks where practical; they are not the primary CI target.

### Will there be Python 3 / modern CPython on classic Amiga?

Not as the goal of *this* repository. CPython 3 on classic 68k would be a different effort; on OS4, [python-amigaos4](https://github.com/geekychris/python-amigaos4) already pursues modern CPython 3. For a Python 3–like experience on 68k today, [MicroPython for Amiga](https://github.com/OoZe1911/micropython-amiga-port) is the practical option. Here the aim remains a solid Python **2.7** that classic Amiga users can build, script with, and extend.

### Can I contribute?

Yes. Code, tests, docs, packaging, and toolchain notes are all welcome and stay open source under the project license. PRs at [GitHub](https://github.com/amigazen/amigapython/).

## Contact

- At GitHub https://github.com/amigazen/amigapython/
- on the web at http://www.amigazen.com/amigapython/ (Amiga browser compatible)
- or email toolkit@amigazen.com

## Acknowledgements

*Amiga* is a trademark of **Amiga Corporation**.

Original Amiga Python by Irmen de Jong, released to the Amiga community via Aminet.

Python is a product of the Python Software Foundation; see LICENSE.md.
