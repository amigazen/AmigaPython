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
