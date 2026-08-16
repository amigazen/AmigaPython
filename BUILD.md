# Build Instructions

Follow these instructions to build AmigaPython.

## ToolKit

Building AmigaPython is most easily achieved if your development environment is setup following the ToolKit standard. See ToolKit.md or https://github.com/amigazen/ToolKit

## Prerequisites

AmigaPython requires:

- VBCC compiler with PosixLib extension
- NDK3.2
- include: assign pointed at NDK headers
- netinclude: assign pointed at Roadshow headers (included in NDK3.2)

`pyexpat` is linked with the bundled `Modules/expat` sources (same as
desktop Python). No `expat.library` is required. Shared-library callbacks
into VBCC were not reliable on classic Amiga (JMP to data).

### AmigaOS APIs on the `amiga` module

Classic AmigaPython folds OS helpers into the builtin `amiga` module (and
a single `Lib/arexx.py`). This maps to AmigaOS 4 Python extras as follows:

| OS4 import | OS3 AmigaPython |
|------------|-----------------|
| `import asl` | `import asl` (shim) or `amiga.FileRequest` / `MessageBox` |
| `import catalog` | `import catalog` or `amiga.OpenCatalog` |
| `import icon` | `import icon` or `amiga.DiskObject` |
| `import arexx` | `import arexx` (`_arexx` accelerator) |
| dos helpers | `import amiga` (former `Doslib` / `Dos.py` / `amigapath`) |

Amiga filesystems are case-insensitive: only one of `arexx.py` /
`ARexx.py` can exist. Use `arexx`.

### Optional shared libraries

| Library | Used by | If missing |
|---------|---------|------------|
| `crc.library` 2+ | `_hashlib` / `hashlib` | `import _hashlib` raises ImportError |
| `asl.library` | `amiga.FileRequest` | RuntimeError on use |
| `locale.library` | `amiga.OpenCatalog` | RuntimeError on use |
| `icon.library` | `amiga.DiskObject` | RuntimeError on use |
| `rexxsyslib.library` | `_arexx` / `arexx` | ARexx APIs unavailable |

The interpreter starts without `crc.library`; install from Aminet when you need hashing.

## Compiler 

The current AmigaPython 2.7.18 requires VBCC

| SAS/C | VBCC | GCC |
|-------|------|-----|
| [ ]   | [X]  | [ ] |

Additional compiler options may be added in the future.

## How To Build

```
Assign Python: Source/
cd Source/
make -f vmakefile
```

This creates a test binary called Python27 in the Source folder.

## How to Clean

```
make -f vmakefile clean
```

## Workbench vs Shell

- **Shell / CLI:** stdin/stdout/stderr stay on the shell console (unchanged).
- **Workbench (icon double-click):** PosixLib has no SAS/C `__stdiowin`. The
  binary opens one `CON:.../CLOSE/SMART` (no WAIT — window closes when the
  FH is closed), clears all `PRF_CLOSE*` flags, and Closes the FH once on
  exit. Link with `-lposix` before `-lvc` (see `aos68k_posix`) so vclib
  fclose does not Close the same CON BPTR for each of stdin/stdout/stderr.
  Needs `lib/python27.zip` beside the tool (copy-anywhere) or optional
  `Assign Python:`. Quit via the CON close gadget or Ctrl+C.
- **Workbench + project icon** (Default Tool = Python): after the same
  console setup, `sm_ArgList[1]` is the script. The port uses
  `CurrentDir(wa_Lock)` + leaf `wa_Name` (no Lock of the project path while
  Workbench still owns startup locks). Extension need not be `.py`.

See `Source/Amiga/wbconsole.c` and RKR M DOS CON-Handler notes.

After a successful build in `Source/`:

```
make -f vmakefile release
```

This copies **public binary release artifacts** into the sibling `Python/`
drawer. AmigaDOS paths use `/Python` (parent). Do **not** pass those paths
into `Python27` — PosixLib treats `/` as volume root, not parent.

| From `Source/` | To `Python/` |
|----------------|--------------|
| `Python27` (stripped release link) | `Python` |
| `lib/lib-dynload/_socket.module` | `lib/lib-dynload/_socket.module` |
| `Lib/site-python/#?.py` | `lib/site-python/` |
| `Lib/` via `mkpythonzip` -> `python27.zip`, then Copy | `lib/python27.zip` |

`python27.zip` is the zipimported stdlib (excludes `test/`, `plat-*`, idle/tk, and other non-Amiga packages). LoadSeg plugins stay outside the zip under `lib/lib-dynload/`.

### Standalone release drawer (`Python/`)

`Python/` is the copy-anywhere end-user product (no installer script):

| Path | Role |
|------|------|
| `Python` | Interpreter binary |
| `Python.help` | AmigaDOS Help text |
| `lib/` | `python27.zip`, `lib-dynload/`, `site-python/` |
| `Help/AmigaPython.guide` | Port guide (AmigaGuide) |
| `Help/DiveIntoPython/` | Dive Into Python (AmigaGuide, GFDL) |
| `Help/Amiga/` | Amiga module notes (reST) |
| `Demo/` | Amiga-checked example scripts |
| `Demo/`, `Icons/` | Examples and icon extras |
| `README`, `DISCL_and_COPYRIGHT` | Quick start and licenses |

Regenerate Dive Into Python guides on a host with Python 3:

```
python3 Source/Tools/amiga/html2amigaguide.py \
  path/to/diveintopython.html Python/Help/DiveIntoPython
```

It does **not** copy `Python27_Debug`, object trees (`build-vbcc`), or documentation from the build. Docs, demos, and icons under `Python/` are maintained in-tree.

`release` depends on `Python27` and `plugins`, so those targets are built first if missing.
