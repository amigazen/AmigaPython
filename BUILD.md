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

## How To Release

The build system is not yet complete and does not include a distribution build. 
