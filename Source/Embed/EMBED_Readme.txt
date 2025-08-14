
Embedding AmigaPython in your own programs
------------------------------------------

This directory shows how to embed the AmigaPython interpreter in your own
application.  The file test.c shows you all that is needed in your C code.

To compile it, you'll have to assign PythonSrc:  to where your Python
source tree is.

Note that AmigaPython's original main.o file has been replaced by a much
smaller version:  AmigaPythonEmbed.c, which contains all necessary
AmigaPython init and exit code.  It exports one function which you should
call even before Py_Initialize (see the code):

	void InitAmigaPython(int argc, char **argv)

Pass the argc and argv parameters from your main() to this function.
Currently, only the executable's name is used from this, no command line
parameters are parsed.  So you can't use the Python command line parameters
with your own program (usually this is no problem because obviously you
want to have your own command line).

When obscure problems occur, have a look at the AmigaPythonEmbed.c file.
Perhaps some things happen here which are unexpected or incompatible with
your own code.


See the test.c code and the SMAKEFILE for details.



Overview of the Python object/library files you must link with:

AmigaPythonEmbed.o
Modules/getbuildinfo.o
Modules/Modules.lib
Parser/Parser.lib
Python/Python.lib
Objects/Objects.lib
Amiga/amigapythonamitcp.lib

lib:scm881nb.lib
lib:scnb.lib

(in this order). (Since 1.5.2, you'll need to compile with DATA=FARONLY,
because the near data section is too large for DATA=NEAR).

SCOPTIONS are identical to the SCOPTIONS in the Python source tree, except
for the path to the GST and the Include/ directory.


You can override Python's $VER version string by declaring one of your own.
See the code.


Problems:  none.  The only problem I'm aware of is that the resulting
executable is 400K+ minimum... (nearly 600 K minimum for Python 1.6)


Irmen de Jong
irmen@bigfoot.com
