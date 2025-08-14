#######
#
#	ConvertFD.py	-	©1996 Irmen de Jong
#
#######
#
#	Usage:	python ConvertFD.py libbasename ...
#
#	for example: python ConvertFD.py gadtools icon utility
#	will generate library module files in RAM: for each of the 3 libraries.
#
#	This program converts Amiga FD files (containing library function
#	descriptions) to Python modules. It parses the FD's and builds
#	correct LVO tables, including the "magic number" describing the
#	function parameters (which register must take a value).
#	The following is generated:
#	- LVO table
#	- Declaration of exception for the module
#	- General 'call' function
#
#	This program is slightly similar to `fd2pragma'.
#
#	You must have FD: assigned to where you keep your FD files.
#	Also, check the version number of the library. Default is 37,
#	which might raise problems with 3rd party libraries.
#
#	$VER: ConvertFD 1.0 (16.2.96)
#
######

default_libver = 37

import regex

# regular expression matching FD:
lvoregx=regex.compile("\([a-zA-Z0-9_]+\)(\([a-zA-Z0-9_,]*\))(\([aAdD0-9,/]*\))")


def fdfile(lib):     return "FD:"+lib+"_lib.fd"
def libname(lib):    return lib+".library"
def capitalize(str): return chr(ord(str[0])&~32)+str[1:]

# convert register string to register number:
def convreg(str):
	if str[0]=='d' or str[0]=='D':
		return 1<<(ord(str[1])-48)
	elif str[0]=='a' or str[0]=='A':
		return 1<<(ord(str[1])-40)
	else:
		raise ValueError,"Illegal register spec"

# convert FD register list to "magic number":
def str2reg(regspec):
	if len(regspec)>1:
		if regspec[0]==',' or regspec[0]=='/':
			return convreg(regspec[1:]) | str2reg(regspec[3:])
		else:
			return convreg(regspec) | str2reg(regspec[2:])
	else:
		return 0

# convert FD file to Python "pragma" file (library module):
def fd2pragma(fdfile):

	offs=0;	cont=1; priv=0;	dict={}

	fdfh=open(fdfile,"r")

	while cont:
		l=fdfh.readline()
		if l: 
			if l[0]=='*':
				continue
			elif l[0]=='#':
				# control func, like ##bias
				if l[2:5]=='end':
					cont=0
				elif l[2:6]=='bias':
					offs=-eval(l[7:])
				elif l[2:8]=='public' :
					priv=0
				elif l[2:9]=='private' :
					priv=1

			else:
				if not priv:
					# public library func
					if lvoregx.match(l) < 0:
						raise ValueError, "wrong FD syntax"

					r=lvoregx.regs
					a,b=r[1]
					c,d=r[3]
					dict[l[a:b]]=(offs,str2reg(l[c:d]))

				offs=offs-6
		else:
			cont=0
	

	fdfh.close()
	return dict

# output the generated LVO dictionary:
def printdict(dict,output):
	output.write("LVO = {\n")

	k=dict.keys()
	k.sort()
	for i in k:
		output.write(repr(i)+":"+repr(dict[i])+",\n")

	output.write("}\n")

# convert. The main function.
def cvt(lib,outfile):

	try:
		dic=fd2pragma(fdfile(lib))
	except IOError:
		print "Can't open FD file",fdfile(lib)
		return

	output=open(outfile,"w")

	output.write("#\n# AMIGA LIBRARY INTERFACE FOR "+libname(lib)+"\n#\n")
	output.write("# Generated from FD file "+fdfile(lib)+" by ConvertFD.py\n#\n\n")
	output.write("# LVO definitions:\n\n")

	printdict(dic,output)

	output.write("\n\nimport amigalibs\nlibname='"+libname(lib)+"'\n\n")
	output.write("lib = amigalibs.openlib(libname,"+`default_libver`+")\n\n")

	output.write("# Exception for this library:\n"+capitalize(lib)+
				 "libError='"+capitalize(lib)+"libError'\n")
	output.write("\n# General libcall interface:\n")
	output.write("def call(func,args):\n")
	output.write("\ttry:\n\t\treturn lib.call(LVO[func],args)\n")
	output.write("\texcept KeyError:\n\t\traise NameError,func+' not found in '+libname\n")

	output.write("\n################ USER CODE FOLLOWS\n\n")
	output.close()


# start if executed as script:

import sys

if __name__ == '__main__':
	if not sys.argv[1:]: # No arguments
		sys.stderr.write('usage: ' + sys.argv[0] + ' libbasename ...\n')
		sys.exit(2)
	for arg in sys.argv[1:]:
		print "Converting",arg,"..."
		cvt(arg,"RAM:"+arg+"lib.py")
