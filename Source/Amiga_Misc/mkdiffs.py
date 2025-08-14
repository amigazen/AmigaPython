#
# This script will check each source directory, and output the differences
# with the original distribution into a DIFF file.
#
# Uses dirdiff (by me) and diff (from SAS)
#
# Assign python_dist: to the location of the original sources.
# Then cd to the root of the Amiga source tree and type
#   'python Amiga_Misc/mkdiffs.py'
#
#         15-Apr-96,  Irmen de Jong.
#

import os
import string
import sys

def_orig='python_dist:'
def_dest='Amiga_Misc/diffs/'

def dodir(dir,orig=def_orig,dest=def_dest):
	outfile=dest+'DIFFS_'+dir
	print "processing",dir,"-->",outfile
	dd=os.popen('dirdiff '+dir+' '+orig+dir+' short u d').readlines()

	out=open(outfile,'w')

	dd.sort()
	dd.reverse()

	for line in dd:
		d=string.split(line)
		if d[1][-2:]=='.o': continue		# skip object files
		if d[0]=='UNQ1:':
			# unique for current path, ADDED
			out.write('ADDED   : '+d[1]+'\n')
		elif d[0]=='UNQ2:':
			# unique for original path, REMOVED
			out.write('REMOVED : '+d[1]+'\n')
		elif d[0]=='DIF1:':
			# difference in files.
			# Use pipe to get output from diff.
			print '   diff ',d[1]
			p=os.popen("diff -l6000 %s/%s %s/%s" % (dir,d[1],orig+dir,d[1]))
			out.writelines(p.readlines())
	out.close()


def run(list):
	print "GENERATING DIFF FILES IN",def_dest
	print "PATH OF ORIGINAL PYTHON DISTRIBUTION IS",def_orig
	for d in list: dodir(d)

if __name__=='__main__':
	run(['Include','Modules','Objects','Parser','Python'])
