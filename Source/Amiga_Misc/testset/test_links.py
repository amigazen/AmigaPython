
import os

TestError = 'FAILED --- link test'

def test(a,b):
	if a!=b: raise TestError

def test_fail(e):
	global fail
	if fail!=e:
		raise TestError,'found err: '+`fail`+' wanted: '+`e`


def remove_files():
 try:
	os.system("delete >NIL: t:#? all quiet force")
	os.unlink("ram:t_hard")
	os.unlink("ram:t_soft")
	os.unlink("ram:dir_soft")
	os.unlink("ram:dir_hard")
 except os.error: pass

if os.getuid()!=0:
	print "Sorry you are not logged in as root. This script cannot perform all link tests. Log in as root and try again."
	raise SystemExit,10

remove_files()

print 'TESTING ISXXX & LINK FUNCTIONS...'
os.link("ram:t","ram:t_hard")
os.symlink("ram:t","ram:t_soft")
os.symlink("c:dir","ram:dir_soft")
fail=0
try: os.link("c:dir","ram:dir_hard")
except os.error,e: fail=e[0]
test_fail(22)	# invalid argument
test(os.path.isdir("ram:t"), 1)
test(os.path.isdir("ram:t_hard"),1)
test(os.path.isdir("ram:t_soft"),1)
test(os.path.isdir("ram:dir_soft"),0)
test(os.path.isfile("ram:dir_soft"),1)
test(os.path.isfile("ram:t_hard"),0)
test(os.path.islink("ram:t_hard"),0)
test(os.path.islink("ram:t_soft"),1)
test(os.path.islink("ram:dir_soft"),1)
test(os.path.islink("ram:dir_sof"),0)

print 'TESTING MKDIR & CIRCULAR LINKS...'
fail=0
try: os.mkdir("ram:dir_soft/foo",0644)
except os.error,e: fail=e[0]
test_fail(22)	# invalid argument
os.mkdir("ram:t_soft/foo",0644)
os.mkdir("ram:t_soft/bar",0000)
os.mkdir("ram:t_soft/foo2",0700)
os.mkdir("ram:t_soft/foo/bar",0070)
os.mkdir("ram:t_soft/foo/bar2",0007)
os.mkdir("ram:t_soft/foo/bar3",0777)
os.symlink("c:dir","ram:t_soft/foo/bar2/dir_soft")
os.symlink("c:dir","ram:t_soft/foo/bar2/dir_soft2")
fail=0
try: os.link("ram:t","ram:t_soft/foo/bar2/t_hard")
except os.error,e: fail=e[0]
test_fail(62)	# ELOOP

fail=0
try: os.symlink("ram:t","ram:t_soft/foo/bar2/t_soft")
except os.error,e: fail=e[0]
test_fail(62)	# ELOOP

print 'TESTING READLINK...'
fail=0
try: os.readlink("ram:dir_sof")
except os.error,e: fail=e[0]
test_fail(2)	# no such file or dir
test(os.readlink("ram:dir_soft"),"c:dir")
test(os.readlink("ram:t_soft"),"ram:t")
fail=0
try: os.readlink("ram:t")
except os.error,e: fail=e[0]
test_fail(22)	# invalid argument
fail=0
try: os.readlink("ram:t_hard"),"error!!"
except os.error,e: fail=e[0]
test_fail(22)	# invalid argument
test(os.readlink("ram:t_soft"),"ram:t")
fail=0
try: os.readlink("c:dir")
except os.error,e: fail=e[0]
test_fail(22)	# invalid argument

remove_files()

print 'LINK TEST OK'
