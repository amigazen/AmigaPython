#######
#
#	ConvertFD.py - convert Amiga FD files to Python LVO tables
#	Based on Irmen de Jong ConvertFD 1.0 (16.2.96); updated for Python 2.7.
#
#	Usage:  python ConvertFD.py libbasename ...
#	Example: python Tools/ConvertFD.py intuition
#
#	Requires FD: assigned to your NDK FD drawer (e.g. FD:intuition_lib.fd).
#	Writes Lib/site-python/<lib>lib.py by default (or -o DIR).
#
#	This is secondary FFI support used with builtin amigalibs.
#	Prefer curated amiga / amigagui APIs when available.
#	Constants: use Tools/scripts/h2py.py on NDK headers separately.
#
######

from __future__ import print_function

import os
import re
import sys

default_libver = 37

lvoregx = re.compile(
	r"^([a-zA-Z0-9_]+)(\([a-zA-Z0-9_,]*\))(\([aAdD0-9,/]*\))"
)


def fdfile(lib):
	return "FD:" + lib + "_lib.fd"


def libname(lib):
	return lib + ".library"


def capitalize(s):
	if not s:
		return s
	return s[0].upper() + s[1:]


def convreg(s):
	c = s[0]
	if c == "d" or c == "D":
		return 1 << (ord(s[1]) - 48)
	if c == "a" or c == "A":
		return 1 << (ord(s[1]) - 40)
	raise ValueError("Illegal register spec")


def str2reg(regspec):
	if len(regspec) > 1:
		if regspec[0] == "," or regspec[0] == "/":
			return convreg(regspec[1:]) | str2reg(regspec[3:])
		return convreg(regspec) | str2reg(regspec[2:])
	return 0


def fd2pragma(path):
	offs = 0
	cont = 1
	priv = 0
	dic = {}
	fdfh = open(path, "r")
	try:
		while cont:
			l = fdfh.readline()
			if not l:
				cont = 0
				break
			if l[0] == "*":
				continue
			if l[0] == "#":
				if l[2:5] == "end":
					cont = 0
				elif l[2:6] == "bias":
					offs = -int(l[7:].strip())
				elif l[2:8] == "public":
					priv = 0
				elif l[2:9] == "private":
					priv = 1
				continue
			if not priv:
				m = lvoregx.match(l)
				if not m:
					raise ValueError("wrong FD syntax: " + l.rstrip())
				name = m.group(1)
				regspec = m.group(3)[1:-1]
				dic[name] = (offs, str2reg(regspec))
			offs = offs - 6
	finally:
		fdfh.close()
	return dic


def printdict(dic, output):
	output.write("LVO = {\n")
	keys = dic.keys()
	keys.sort()
	for i in keys:
		output.write("    %r: %r,\n" % (i, dic[i]))
	output.write("}\n")


def cvt(lib, outfile):
	path = fdfile(lib)
	try:
		dic = fd2pragma(path)
	except IOError:
		print("Can't open FD file", path)
		return 0

	output = open(outfile, "w")
	try:
		output.write("#\n# AMIGA LIBRARY INTERFACE FOR " + libname(lib) + "\n")
		output.write("# Generated from FD file " + path + " by ConvertFD.py\n")
		output.write("# Secondary FFI: use with amigalibs. Prefer curated amiga APIs.\n#\n\n")
		output.write("# LVO definitions:\n\n")
		printdict(dic, output)
		output.write("\n\nimport amigalibs\n")
		output.write("libname = %r\n\n" % libname(lib))
		output.write("lib = amigalibs.openlib(libname, %d)\n\n" % default_libver)
		output.write("# Exception for this library:\n")
		output.write(capitalize(lib) + "libError = '" +
			capitalize(lib) + "libError'\n\n")
		output.write("# General libcall interface:\n")
		output.write("def call(func, args):\n")
		output.write("    try:\n")
		output.write("        return lib.call(LVO[func], args)\n")
		output.write("    except KeyError:\n")
		output.write("        raise NameError, func + ' not found in ' + libname\n")
		output.write("\n################ USER CODE FOLLOWS\n\n")
	finally:
		output.close()
	return 1


def main(argv=None):
	if argv is None:
		argv = sys.argv[1:]
	outdir = None
	args = []
	i = 0
	while i < len(argv):
		if argv[i] == "-o" and i + 1 < len(argv):
			outdir = argv[i + 1]
			i = i + 2
			continue
		args.append(argv[i])
		i = i + 1
	if not args:
		sys.stderr.write(
			"usage: %s [-o outdir] libbasename ...\n" % sys.argv[0])
		return 2
	if outdir is None:
		here = os.path.dirname(os.path.abspath(sys.argv[0]))
		# Tools/ -> Source/Lib/site-python
		root = os.path.dirname(here)
		outdir = os.path.join(root, "Lib", "site-python")
	if not os.path.isdir(outdir):
		sys.stderr.write("output directory missing: %s\n" % outdir)
		return 1
	for arg in args:
		out = os.path.join(outdir, arg + "lib.py")
		print("Converting", arg, "->", out, "...")
		if not cvt(arg, out):
			return 1
	return 0


if __name__ == "__main__":
	sys.exit(main())
