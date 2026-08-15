
Embedding AmigaPython 2.7.18 in your own programs
-------------------------------------------------

This directory shows how to embed the AmigaPython interpreter in your own
application.  The file test.c shows what is needed in your C code.

Original embed support was written for classic AmigaPython by Irmen de Jong;
this tree targets Python 2.7.18 (see SMAKEFILE).


Build notes
-----------

Assign PythonSrc: (or adjust SMAKEFILE paths) to your AmigaPython Source
tree root, then build from this Embed/ directory with SAS/C `smake` using
the local SMAKEFILE / SCOPTIONS.

For the main interpreter build (VBCC), see ../README.AMIGA and BUILD.md.
The Embed demo here remains oriented around the SAS/C link libraries
produced by that toolchain path.


InitAmigaPython
---------------

AmigaPythonEmbed.c replaces a full Modules/main.o for embedded apps.  It
contains AmigaPython init and exit code and exports:

	void InitAmigaPython(int argc, char **argv)

Call this even before Py_Initialize (see the code).  Pass argc and argv
from your main().  Currently only the executable name is taken from argv;
Python's own command-line flags are not parsed (usually fine — you want
your application's command line).

When obscure problems occur, inspect AmigaPythonEmbed.c for Amiga-specific
startup (assigns, networking library checks, etc.) that may interact with
your own code.


See test.c and SMAKEFILE for details.


Overview of object/library files you must link with (SAS/C style):

AmigaPythonEmbed.o
Modules/getbuildinfo.o
Modules/Modules.lib
Parser/Parser.lib
Python/Python.lib
Objects/Objects.lib
expat/lib/expat.lib          (if your Modules.lib expects it)
Amiga/amigapythonamitcp.lib  (or the no-net equivalent for your build)

lib:scm881nb.lib
lib:scnb.lib

(in this order).  Large near-data forced DATA=FARONLY on classic SAS/C
builds; keep SCOPTIONS consistent with the main Source tree.

SCOPTIONS here match the Source tree except for GST / Include paths.


You can override Python's $VER version string by declaring one of your own.
See the code.


Size: embedded executables are large (historically 400K+; expect more for
2.7.18 with a fuller library).  Strip debug and drop unused modules in
Modules/config.c when footprint matters.


Report AmigaPython embed issues via amigazen project.
