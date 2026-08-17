# AmigaPython 2

This is AmigaPython 2, a port of **Python 2.7.18** to Amiga - continuing Irmen de Jong's AmigaPython work on his original Aminet AmigaPython releases 1.4, 1.5 and 2.0.

## [amigazen project](http://www.amigazen.com)

*A web, suddenly*

*Forty years meditation*

*Minds awaken, free*

**amigazen project** is using modern software development tools and methods to update and rerelease classic Amiga open source software. Projects include a new AWeb, a new AmigaPython 2, and the ToolKit project - a universal SDK for Amiga.

Key to the amigazen project approach is ensuring every project can be built with the same common set of development tools and configurations, so the ToolKit project was created to provide a standard configuration for Amiga development. All *amigazen project* releases will be guaranteed to build against the ToolKit standard so that anyone can download and begin contributing straightaway without having to tailor the toolchain for their own setup.

The original authors of *Python* and of the classic Amiga port are not affiliated with amigazen project. This software is redistributed on terms described in the documentation, particularly the file LICENSE.md

The amigazen project philosophy is based on openness:

*Open* to anyone and everyone	- *Open* source and free for all	- *Open* your mind and create!

PRs for all projects are gratefully received at [GitHub](https://github.com/amigazen/). While the focus now is on classic 68k software, it is intended that all amigazen project releases can be ported to other Amiga-like systems including AROS and MorphOS where feasible.

## About AmigaPython

Python was first ported to and adapted for the Amiga in 1999-2000 by **Irmen de Jong**. His Aminet releases through Python 2.0 remain in this repository's history and `archive/` folder.

This project continues that work by bringing AmigaPython up to **Python 2.7.18** (a port of the very last CPython 2 release, April 2020), built against the ToolKit SDK on a classic Amiga.

AmigaPython aims to be as complete a port as makes sense on AmigaOS 3.x: the usual 2.7 language and library surface, plus Amiga-native modules (`amiga`, `arexx`, ASL / catalog / icon helpers, and related site-python shims).

This project is **not** the Python that ships with OS4 but does implement the same Amiga native Python modules and functions. See the FAQ below. Collaboration on OS4 and other Amiga-like platforms is welcome in the amigazen project spirit of openness.

### Goals

- A **ToolKit-buildable** Python 2.7 for classic 68k Amiga using VBCC and PosixLib
- Networking against modern **bsdsocket.library** (e.g. Roadshow)
- Amiga integration that feels native: AmigaDOS / ASL / catalogs / icons / ARexx, documented in Library Reference-style reST

Official upstream docs: https://docs.python.org/2.7/

## What's new in Python 2.x (2.0 - 2.7.18)

Relative to the classic AmigaPython **2.0** baseline, upstream Python 2 gained roughly a decade of language and library work. Highlights (see also https://docs.python.org/2.7/whatsnew/):

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
| Data & CLI | `json`, `argparse`, `collections` (`OrderedDict`, `Counter`, etc.) |
| Crypto & process | `hashlib` / `hmac`, `subprocess` |
| Functional | `itertools`, `functools`, `contextlib`, `abc` |
| Apps | `logging`, stronger `unittest`, `email` overhaul |
| Net & XML | `urllib2` / `httplib` evolution; ElementTree + Expat |
| Packaging | Prefer current 2.7 module names in new code |

## What's new in this Amiga port

### Amiga-specific improvements


| Item | Notes |
|------|--------|
| Toolchain | **VBCC + PosixLib** / NDK 3.2 |
| Networking | **bsdsocket.library** (Roadshow and friends); I-Net225 no longer supported |
| I/O | Builtin **`_io`** so `io` / `tempfile` work (Amiga-friendly text path when Unicode is off) |
| Packaging | `zipimport` + `lib/python27.zip`-style stdlib; `_socket` and `_ssl` as dynload plugins |
| `amiga` | Expanded POSIX-style surface (`access`, `sleep`/`usleep`, `waitpid`, `pathconf`/`sysconf`, `urandom`, `fsync`, `closerange`, etc.) |
| Dos helpers | Former Doslib / `_amigados` (`ArgParser`, `FIBF_*`, `touch`, Assign helpers) on `amiga` |
| Paths | `amiga.to_unix` / `amiga.from_unix` |
| Requesters | ASL `FileRequest` / `MessageBox`; locale `OpenCatalog`; icon `DiskObject` |
| site-python | OS4-style shims: `arexx`, `asl`, `catalog`, `icon`, `amigavars`, etc.|
| ARexx | `arexx` / `_arexx` high-level support retained and updated |
| Tests | AmigaTests coverage for Amiga extras and related builtins |

### Standalone release drawer

The shippable end-user product is the `Python/` drawer: copy it anywhere and run, optionally assigning the path "Python:" to wherever Python is installed.
There is no longer any need for an Installer script. Libraries live in `lib/` next to the Python binary. Docs include `Help/Python.help`,
`Help/AmigaPython.guide`, and `Help/DiveIntoPython/` (GFDL AmigaGuide of the classic
Dive Into Python text for Python 2). 

## About ToolKit

**ToolKit** exists because most Amiga software was written in the 1980s and 90s by individuals, each with their own layout for compilers, includes, and libs. Open source collaboration did not look like it does in 2026.

**ToolKit** from amigazen project is a work in progress toward a standardised Native Developer Kit plus compilers, build tools, and third-party components so projects can be built together without every contributor rewriting makefiles for their private toolchain.

All *amigazen project* releases ship in a ready-to-build configuration according to the ToolKit standard. Each ToolKit component is open source with its own GitHub repo; ToolKit itself will eventually be an easy install of redistributable pieces plus scripts for parts that cannot be freely redistributed.

## Other Python projects on Amiga

Several independent efforts bring Python (or a Python-like language) to Amiga-family systems. They solve different problems, and other ports may serve your needs better.

| | **This project** (AmigaPython 2) | [python-amigaos4](https://github.com/geekychris/python-amigaos4) | [micropython-amiga-port](https://github.com/OoZe1911/micropython-amiga-port) |
|--|--|--|--|
| **What it is** | CPython **2.7.18** continued from Irmen's AmigaPython | CPython **3.12.7** for AmigaOS 4 | **MicroPython** v1.28 (Python 3-like subset runtime) |
| **Platform** | Classic **68k** AmigaOS 3.x | **PowerPC** AmigaOS 4.1 (sam460ex / QEMU / hardware) | Classic **68020+** AmigaOS (and emulators) |
| **Lineage** | Aminet AmigaPython 2.0 updated to CPython 2.7 | New OS4 CPython port (not this repo) | Separate MicroPython port |
| **Build** | Native Amiga / ToolKit (**VBCC + PosixLib**; SAS/C where useful) | Cross-compile from Linux/macOS via **Docker** (GCC) | Cross-compile with **bebbo m68k-amigaos-gcc** |
| **Language model** | Python 2 (`str` as bytes; Unicode optional / often off) | Full Python 3 (Unicode-centric) | MicroPython dialect (f-strings, async) |
| **Stdlib / packaging** | 2.7 library + zipimport; Amiga site-python | Broad 3.12 stdlib; **pip** for pure-Python wheels | Curated MicroPython modules + `.mpy` bytecode |
| **Networking** | bsdsocket; `_socket` as LoadSeg plugin | bsdsocket; SSL via optional AmiTLS | bsdsocket; HTTPS / SMTP via AmiTLS where enabled |
| **Amiga APIs** | `amiga`, ARexx, ASL, catalogs, icons, etc. | `_amiga` + `amiga.*` (Exec/DOS/Intuition/...) | `amiga.intuition`, `amiga.asl`, `arexx`, ... |
| **Best when you want** | Full CPython 2 with Amiga native modules | Modern Python 3 and PyPI on **OS4** | Small footprint, REPL, and Python 3-flavoured scripts on **68k** |

amigazen project's AmigaPython 2 is deliberately the **classic CPython 2** line. For Python 3 on OS4, see geekychris's port; for a lean 68k interpreter with a Python 3-like surface, see OoZe1911's MicroPython port. Ideas and friendly cross-pollination are welcome; the codebases are not drop-in substitutes.

## Frequently Asked Questions

### Why Python 2 and not Python 3?

Because this project continues **Irmen de Jong's AmigaPython**, which was a Python **2** port, and because classic Amiga is a constrained 68k / AmigaOS 3 environment.

Python 3 assumes a Unicode-centric runtime, a larger standard library, and a toolchain/runtime budget that does not map cleanly onto typical classic Amiga hardware and SDKs. Python **2.7.18** is the last 2.x release: it still matches the classic port's model (`str` as bytes, optional Unicode), gives nearly a decade of language and library improvements over AmigaPython 2.0, and remains a realistic target for classic hardware limitations and C compilers.

### Which key language features and modules *are* included?

This is **CPython 2.7.18** with an Amiga-oriented builtin set. Default `make -f vmakefile` links both **`Python`** (`BUILD=standard`, the full C module set) and **`SlimPython`** (Amiga native modules and stdlib boot; omits the *standard-only* lines; named after Irmen de Jong's original SlimPython 1.5.2).

**Language / runtime (2.7)**

- New-style classes, descriptors, `property`, `super`, decorators, `with` / context managers
- Generators (`yield`), nested scopes, list/dict/set comprehensions, `str.format()`
- Exceptions, `gc`, weakrefs, `ast` / `_ast`, warnings
- Primary text type is 8-bit `str` (see Unicode FAQ); `_io` is built so `io` / `tempfile` work without a full Unicode build

**Shared builtins** (both builds)

- `array`, `math`, `time`, `operator`, `strop`, `_struct` / `struct`
- `binascii`, `cStringIO`, `cPickle`, `marshal`, `_sre` / `re`, `_codecs` (subset - see below)
- `_collections`, `itertools`, `_functools`, `_random` / `random`
- `zlib`, `zipimport`, `select`, `errno`, `imp`, `_symtable`, `_weakref`, `_warnings`

**Standard-only builtins** (`BUILD=standard`)

- `datetime`, `cmath`, `_bisect`, `_heapq`, `_csv`
- `md5`, `sha`, `_hashlib` (Amiga `crc.library` where wired)
- `pyexpat` / bundled Expat XML
- `pwd` / `grp` / `crypt` / `syslog` (AmiTCP / usergroup)

**OS / Amiga** (both builds)

- Builtin `amiga` (POSIX-style OS API used via `os`), plus AmigaDOS / ASL / catalog / icon helpers
- `environment`, `_arexx` / `arexx`, `amigagui`, `amigalibs`
- `Lib/site-python` shims (`asl`, `catalog`, `icon`, `amigavars`, legacy `dos`, etc.)
- Pure-Python 2.7 stdlib on disk / zip where packaged (`os`, `os.path`, `json`, `argparse`, `email`)

**Networking (optional at runtime)**

- `_socket` as a **LoadSeg plugin** (not a startup builtin): needs `bsdsocket.library`; interpreter starts without opening the stack
- Standard build only: builtins `pwd` / `grp` use PosixLib stubs without usergroup; `crypt` / `syslog` open `usergroup.library` / require a live `SocketBase` and raise `SystemError` if missing (no hard crash through a dummy library base)

### Which key language features and modules are *not* included (or not default)?

These are the important gaps relative to a full CPython 2.7:

**Language / runtime**

- **Full Unicode** off by default (`Py_USING_UNICODE` undefined): no wide `unicode` type as the primary text model; `unicodedata` is not built in
- **No threads** (`WITH_THREAD` undefined): no `thread` / `threading` as on Unix CPython
- **Incomplete codecs** when Unicode is off - see the FAQ below

**OS builtin naming (`amiga` vs `posix`)**

- On Unix, `import os` re-exports the builtin named **`posix`**.
- On AmigaPython, **`posixmodule.c` is not compiled**. The OS builtin is **`amiga`**; `os.py` does `from amiga import *`.
- After `amiga` is initialized, **`sys.modules['posix']` is set to the same module** so naive `import posix` still works. Portable code should keep using **`import os`**. Use `import amiga` for Amiga-only APIs (ASL, catalogs, Dos helpers, ...).
- Do not add `posix` to `config.c`'s inittab: `os.py` checks for builtin name `posix` before `amiga`, and would take the wrong bootstrap path.

**C extensions typically absent or not shipping as builtins**

- `_ssl` is a **LoadSeg plugin** (`lib/lib-dynload/_ssl.module`), AmiTLS-backed; AmiSSL is compiled only as an alternate and is not the release default
- `_ctypes`, `_sqlite3`, `_multiprocessing`, `_tkinter`, `_bsddb`, `_hotshot` / `_lsprof` profilers
- `mmap`, `bz2`, `audioop`, `parser` (pgen), and many other optional Unix modules

**Platform / packaging**

- Not a drop-in for scripts that assume Unix processes, fork, full POSIX signals, or Windows APIs
- **I-Net225** networking is unsupported; use modern **bsdsocket** (Roadshow, UAE net, ...)
- `pip` / binary wheels are not a supported distribution story on classic 68k the way they are on OS4's CPython 3

If a pure-Python stdlib module imports a missing C accelerator, that feature fails even though the `.py` file may be on the tree. Prefer AmigaTests and the Amiga docs under `Python/Help/Amiga/` for what is actually exercised.

### Why is the codec / `str.encode` story incomplete?

The codec machinery is only half there in the default Amiga build, because Unicode is off.

**How CPython 2 normally does it**

Calls such as `"abc".encode("ascii")` or `u"cafe".encode("utf-8")`, and much of XML / ElementTree I/O, go through:

1. the pure-Python `encodings` package, which
2. calls into the builtin **`_codecs`** module for the real work (`ascii_encode`, `utf_8_encode`, `latin_1_encode`, etc.).

Those `_codecs.*_encode` / `*_decode` helpers are implemented with the Unicode C API (`PyUnicode_FromObject`, `PyUnicode_EncodeASCII`, etc.). In stock `_codecsmodule.c` they sit behind `#ifdef Py_USING_UNICODE`.

**What this port does**

Default builds have `#undef Py_USING_UNICODE`. At compile time those helpers are **not** entered into `_codecs`'s method table. You still get a `_codecs` module (registry / lookup stubs), but **not** the usual C encoders.

So `"abc".encode("ascii")` fails when the encodings layer looks for `_codecs.ascii_encode` (AmigaTests skip with *"ascii_encode not in _codecs yet"*). **ElementTree** often hits the same wall even though **`pyexpat`** works for lower-level XML.

**What still works**

- Plain 8-bit `str` operations that never ask for an encoding
- `_io` text paths shimmed for no-Unicode (identity / latin-1-ish; skip full codec lookup)
- `pyexpat` when you stay on the Expat API and avoid ElementTree's encode path

**"Until more `_codecs` encoders are wired"** means either turning Unicode on (expensive on classic Amiga), or adding 8-bit-only C stubs for common encodings (`ascii`, `latin-1`, maybe UTF-8 as bytes<->bytes) that do not need `PyUnicode_*`. Until then: registry present, real encode/decode C builtins largely absent.

### Why doesn't AmigaPython support Unicode (by default)?

Full CPython Unicode (`Py_USING_UNICODE`, wide builds, `unicodedata`, and the text stack that assumes them) is expensive in code size, RAM, and complexity on classic Amiga. This port often builds **without** `Py_USING_UNICODE` so the interpreter and `_io` stay usable; text is primarily **8-bit `str`** (Latin-1 / locale-ish), which matches how most classic Amiga software already treats strings.

The Unicode enabled build option is currently untested, with no substrate that provides Unicode/UTF-8 support on Amiga (codesets.library could be a future option).

MicroPython on Amiga and CPython 3 on OS4 take different Unicode choices because they target different runtimes and hardware budgets.

### What compiler is used to build AmigaPython?

VBCC version 0.9h is used to build AmigaPython, together with the VBCC PosixLib. Earlier versions of AmigaPython used SAS/C but the more recent versions of CPython ported here use complex patterns of C, especially in the type objects implementations, that SAS/C could not compile successfully. By contrast, VBCC has been able to compile large parts of the CPython codebase unmodified, as well as having a richer, more POSIX compliant C library in the form of PosixLib. In this way, AmigaPython also serves as an excellent test for all the features of VBCC.

VBCC allows for both hosted compilation on an Amiga, or cross compiler on another VBCC compatible environment, using the m68k-amigaos target.

Artifacts from the SAS/C build including smakefile and SCOPTIONS files remain in the project source code in case it may be possible to build AmigaPython using SAS/C again one day, by refactoring the problematic sections.

### What functionality is PosixLib used for, versus Amiga native functionality, in AmigaPython?

PosixLib is the C runtime layer that lets unmodified CPython C call `open` / `read` / `malloc` and similar libc. Amiga-native code is used wherever POSIX is the wrong model, would deadlock Workbench, or (for sockets) would break SSL.

**PosixLib (and vclib) provide**

- libc and stdio (`FILE *`, `malloc`, string/math) so CPython objects and `_io` can stay close to stock sources
- a POSIX fd table (`__fdesc[]`) over AmigaDOS **file** handles, plus `select()` on those file fds
- opening `bsdsocket.library` once (`__init_bsdsocket`, process-wide `SocketBase`) so the host and syslog / `crypt` share one library base
- DNS helpers still used by `_socket` (`gethostbyname`, `inet_*`); those stay PosixLib
- path spelling that CPython expects (`/` separators in some helpers); `getpath` / `os.path` still speak Amiga `volume:drawer/file`

**Not PosixLib: `_socket` / `_ssl` I/O (psockets)**

PosixLib's own `socket()` / `bind()` / `recv()` wrappers were **bypassed**. Those fds did not work with AmiTLS (`TlsRead` returned 8808 / `EWOULDBLOCK`). The host implements **psockets** in `Amiga/pyamiga_host.c`: AmiTCP LVOs under a private posix-like fd table (`pysock_*`). Python `fileno()` is a slot >= 3 (never 0/1/2). Connect, send, recv, ioctl, and `select` on sockets go through those trampolines, not PosixLib `__P*` socket macros. AmiTLS / AmiSSL get the **AmiTCP native** id via `fn_socket_native_fd()`, not the Python slot and not a PosixLib `__fdesc` entry.

**Amiga-native (exec / dos / intuition / icon, and the `amiga` builtin) provide**

- the OS module itself: `posixmodule.c` is not compiled; `os` is instead provided by a native amiga module implementing the posix/os module API `from amiga import *` (with a `sys.modules['posix']` alias)
- Workbench startup: `GetMsg` of `WBStartup` and a CON: console allow for launching scripts or the interactive interpreter from Workbench
- requesters, catalogs, icons, ARexx can be used via Amiga native Python modules
- hashing via `crc.library` (`_hashlib`), locale via `locale.library`, optional `usergroup.library` for `pwd`/`grp`

### How do the dynamic plugins for _socket and _ssl work and why isn't this technique used for all binary modules?

Classic Amiga has no `dlopen`. A C extension is either linked into the Python runtime binary (`Modules/config.c`) or loaded later with `LoadSeg` as `lib/lib-dynload/_name.module`.

`_socket` and `_ssl` use the second path (ABI in `Include/pyamiga_plugin.h`, loader `Python/dynload_amiga.c`, host vtable `Amiga/pyamiga_host.c`). On `import _socket` Python LoadSegs the image, checks a `PyAm` module head (Workbench-safe `MOVEQ #-1; RTS` so the file cannot be "run"), then calls `entry(PyHost *)`. The plugin is built with `-nostdlib` and does not link PosixLib: every Python C API, malloc, and socket call is provided a function pointer into the **host** interpreter.

Socket I/O is **psockets** on the host: `socket()` in `_socket` allocates a posix slot over an AmiTCP LVO, never PosixLib `socket()` - this was necessary as PosixLib's socket fd's cannot be wrapped with SSL connections. That keeps one heap and one `SocketBase`. `_ssl` attaches AmiTLS with `fn_socket_native_fd()` (the bsdsocket id).

This is **not** used for every binary module:

- Startup-critical code (`zipimport`, `zlib`, `_io`, `_sre`, `_codecs`, `amiga`, `select`, `errno`) must be in the binary before any `.module` can load.
- Modules with `static PyTypeObject` (tp_name in the plugin image) or C callbacks into a second library (stock `pyexpat` + `expat.library`) are unsafe as LoadSeg plugins on this ABI. Those stay linked, or are omitted in **SlimPython**.
- Each new plugin needs host trampolines, an ABI bump, and 680x0 calling-convention care (scratch registers, no plugin-side `PyString_AS_STRING` on host objects).

So plugins are for **optional, heavy, late-opened** libraries (`bsdsocket.library`, `amitls.library`). The rest of the C accelerator set is either always linked or left out of SlimPython. See `Docs/PLUGIN_GUIDE.md`.

### Why does the ssl module only support AmiTLS and not AmiSSL?

Support for both AmiTLS and AmiSSL is a build configuration flag in the _ssl module implementation. However the AmiSSL build does not pass the unit tests, while the AmiTLS version does.

### Why does AmigaPython only support bsdsocket.library?

socket.library as used in the as225, I-Net225 and Genesis products has not been maintained for over 25 years.

In contrast, a supported implementation in the form of Olaf Barthel's Roadshow is readily available, while UAE variants also usually contain a bsdsocket.library.

### Why don't you use expat.library as a substrate for the pyexpat module, and dynamically load pyexpat only when needed?

This was tested during development but currently the callbacks needed for working with the shared library version of expat do not work correctly with AmigaPython and the stock pyexpat module, leading to fatal crashes. This may be revisited in a future release.

### Why don't you use z.library as a substrate for the zip module, and dynamically load zip modules?

This has also been tested during development, and remains a possible future optimisation. However, embedded zlib allows for core functionality such as zipimport of the Python Standard Library modules on startup.

### How does Python work with ARexx? Could Python replace ARexx as the system scripting language?

Python **talks to** ARexx; it does not replace RexxMast or the `RX` command.

The builtin `_arexx` module uses `rexxsyslib.library`. Public code should `import arexx` (`Lib/site-python/arexx.py`). From Python you can send commands to any ARexx host (`arexx.dorexx("WORKBENCH", "...")`, Directory Opus, AWeb, the `REXX` port, and so on) and sync stem variables from a dict. You can also **be** a host: `arexx.publicport` / `arexx.host` (default port name `PYTHON`) so ARexx scripts and other apps can `ADDRESS PYTHON` and dispatch commands with `amiga.ArgParser` templates. If `rexxsyslib.library` is missing, those APIs are unavailable.

Python is not a drop-in system scripting language in ARexx's place, not least because the Python language runtime is several orders of magnitude bigger than the ARexx core.

### What is Dive Into Python?

[Dive Into Python](http://diveintopython.org/) is Mark Pilgrim's Python 2 tutorial (edition 5.4, 2004). It is licensed under the GNU Free Documentation License 1.1+.

The release drawer ships a **modified AmigaGuide edition** under `Help/DiveIntoPython/` (not the original HTML). Language chapters still apply to this 2.7 port. For Amiga setup use `Help/AmigaPython.guide` and the release `README`. 

### Is this the same as OS4 Python / python-amigaos4?

**No.** This repository is the **classic Amiga / AmigaPython** line updated to 2.7.18 under amigazen project.

- Stock or third-party **AmigaOS 4** Python installs, and [geekychris/python-amigaos4](https://github.com/geekychris/python-amigaos4) (CPython 3.12 on PPC OS4), are separate lineages, ABIs, and packaging.
- Some site-python names here are shaped for familiar OS4-style usage (`arexx`, `asl`, `catalog`, `icon`, etc.) where that helps Amiga programmers, but this does not replace OS4 Python (currently at version 2.5) and is not affiliated with Hyperion or with python-amigaos4.

See **Other Python projects on Amiga** above for a side-by-side comparison.

### Why doesn't AmigaPython support tkinter or other popular GUI modules?

`_tkinter` is a C binding to **Tcl/Tk**. There is no supported Tcl/Tk toolkit for classic AmigaOS 3.x in this toolchain, and Tk expects a Unix/X11 or Windows event loop, threads, and a large shared library. Linking `_tkinter.c` would still leave nothing to talk to. 

Other popular CPython GUI bindings (`wxPython`, `PyGTK`, `PyQt`) likewise need foreign toolkits that are not part of AmigaOS.

For native UI this port uses Amiga APIs instead:

- `amigagui` -- simple Intuition windows
- `amiga.FileRequest` / `MessageBox` (ASL)
- `amiga.DiskObject` (icon.library)
- site-python `intuition` / `intuitionlib` helpers

Those are 68k-sized and do not pull Tcl. A full Tk port would be a separate project.

### What is crc.library and what is it used for?

[`crc.library`](https://github.com/amigazen/crc.library) from amigazen project is a shared library that implements digest algorithms (MD5, SHA-1, SHA-256) as Exec LVOs (`DoMD5Sum`, `DoSHA1`, `DoSHA256`).

AmigaPython's builtin `_hashlib` (`Modules/_hashcrc.c`) wraps those LVOs so `import hashlib` matches enough of CPython's OpenSSL-backed `_hashlib` for `hashlib.md5` / `sha1` / `sha256`. The interpreter does **not** open the library at startup: `OpenLibrary("crc.library", 2)` happens on `import _hashlib`. If the library is missing, that import raises `ImportError` and fails gracefully; `md5` / `sha` builtins in the standard build still work without it.

Install `crc.library` 2+ (LIBS:) when you want `hashlib`. It is optional for running scripts that never hash.

### How does this AmigaPython release relate to MicroPython on Amiga?

[micropython-amiga-port](https://github.com/OoZe1911/micropython-amiga-port) is a **MicroPython** port to classic 68k - a different interpreter (not CPython), with a smaller footprint and a Python 3-flavoured language surface. It is excellent when you want a lean REPL and curated modules on an A1200-class machine.

### How does this relate to the AmigaPython 2.0 and earlier releases found on Aminet?

This tree is the direct continuation of the Irmen de Jong AmigaPython port: same Amiga-first spirit, updated to the **2.7.18** language version, with all-new bsdsocket.library and AmiTLS support, and many more new features. Irmen's Aminet packages (through Python 2.0) are the historical baseline. Sources and notes from that era live in the project repository's `archive/` directory. 

Historic package notes: [Aminet Python20](https://www.aminet.net/package/dev/lang/Python20). Contact details in that era readme are obsolete - use the contacts below.

### What AmigaOS versions does it target?

Built and tested with **NDK 3.2** / ToolKit expectations for AmigaOS 3.x. Networking expects a **bsdsocket.library** stack (Roadshow, UAE net, etc.) to be able to use socket module, and **AmiTLS** for ssl module (AmiSSL support is currently too unstable and is not enabled in the release build). _hashlib functions depend on crc.library also from amigazen project.

### Will there be Python 3 / modern CPython on classic Amiga?

Not as the goal of *this* repository. CPython 3 on classic 68k would be a different effort; on OS4, [python-amigaos4](https://github.com/geekychris/python-amigaos4) already pursues modern CPython 3. For a Python 3-like experience on 68k today, [MicroPython for Amiga](https://github.com/OoZe1911/micropython-amiga-port) is the practical option. AmigaPython remains a solid Python **2.7** that classic Amiga users can build, script with, and extend.

### Can I contribute?

Yes. Code, tests, docs, packaging, and toolchain notes are all welcome and stay open source under the project license. PRs at [GitHub](https://github.com/amigazen/amigapython/).

## Contact

- At GitHub https://github.com/amigazen/amigapython/
- on the web at http://www.amigazen.com/amigapython/ (Amiga browser compatible)
- or email toolkit@amigazen.com

## Acknowledgements

*Amiga* is a trademark of **Amiga Corporation**.

Original AmigaPython by Irmen de Jong, released to the Amiga community via Aminet.

Python is a product of the Python Software Foundation; see LICENSE.md.
