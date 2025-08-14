# Module 'amigapath' -- common operations on AMIGA pathnames
# Irmen de Jong, april 1st, 1996 (no joke)
# (adapted from `posixpath.py')
#
# 26-mar-96: fixed split, islink. Improved expanduser.
# 1-apr-96: fixed expanduser (works 100% now)
# 25-dec-96: slight changes to comments and expanduser.
#

import amiga
import stat
import string

# normalize the case of a pathname
normcase = string.lower


# Return wheter a path is absolute. On Amiga: when a ':' occurs in it.

def isabs(s):
	return ':' in s


# Join two pathnames.
# Ignore the first part if the second part is absolute.
# Insert a '/' unless the first part is empty or already ends in '/' or ':'.

def join(a, b):
	if ':' in b: return b
	if a == '' or a[-1:] == '/': return a + b
	# Note: join('x', '') returns 'x/'; is this what we want?
	if a[-1:]==':': return a+b
	return a + '/' + b


# Split a path in head (everything up to the last '/' or ':') and tail (the
# rest).  If the path ends in '/' or ':', tail will be empty.  If there is no
# '/' or ':' in the path, head  will be empty.
# DIFFERENCE WITH posixpath: only ONE trailing '/' will be stripped from head!
# (on the Amiga a double slash means "parent dir"! ) This means that if
# head ends in a '/', you MUST add a '/' to it when reconstructing the path,
# or you will lose the "parent dir" slash.
# Functions that depend on this function are also affected!
#  (basename, dirname)
#
# Suggested by Kent Polk, kent@eaenki.nde.swri.edu

def split(p):
	i = string.rfind(p, '/')
	j = string.rfind(p, ':')
	if (i>j) and p[-1]=='/':
		p=p[:-1]
		i = string.rfind(p, '/')
	if (j>i) or ((j>=0) and (i<0)): i=j
	head, tail = p[:i+1], p[i+1:]
	if head:
		if head[-1]=='/': head=head[:-1]
	return head, tail


# Split a path in root and extension.
# The extension is everything starting at the last dot in the last
# pathname component; the root is everything before that.
# It is always true that root + ext == p.

def splitext(p):
	root, ext = '', ''
	for c in p:
		if c == '/':
			root, ext = root + ext + c, ''
		elif c == '.':
			if ext:
				root, ext = root + ext, c
			else:
				ext = c
		elif ext:
			ext = ext + c
		else:
			root = root + c
	return root, ext


# Split a pathname into a drive specification and the rest of the
# path.  Useful on DOS/Windows/NT/Amiga; on Unix, the drive is always empty.

def splitdrive(p):
	i = string.rfind(p,':') + 1
	if i<=0: return '', p
	return p[:i],p[i:]


# Return the tail (basename) part of a path.

def basename(p):
	return split(p)[1]


# Return the head (dirname) part of a path.

def dirname(p):
	return split(p)[0]


# Get the full pathname of a file/directory (i.e. expand assigns).

fullpath = amiga.fullpath


# Return the longest prefix of all list elements.

def commonprefix(m):
	if not m: return ''
	prefix = m[0]
	for item in m:
		for i in range(len(prefix)):
			if prefix[:i+1] <> item[:i+1]:
				prefix = prefix[:i]
				if i == 0: return ''
				break
	return prefix


# Is a path a symbolic link?
# This will always return false on systems where amiga.lstat doesn't exist.
# (or where S_ISLNK isn't defined)
def islink(path):
	try:
		st = amiga.lstat(path)
		return stat.S_ISLNK(st[stat.ST_MODE])
	except (amiga.error, AttributeError):
		return 0


# Does a path exist?
# This is false for dangling symbolic links.

def exists(path):
	try:
		st = amiga.stat(path)
	except amiga.error:
		return 0
	return 1


# Is a path an amiga directory?
# This follows symbolic links, so both islink() and isdir() can be true
# for the same path.

def isdir(path):
	try:
		st = amiga.stat(path)
	except amiga.error:
		return 0
	return stat.S_ISDIR(st[stat.ST_MODE])


# Is a path a regular file?
# This follows symbolic links, so both islink() and isfile() can be true
# for the same path.

def isfile(path):
	try:
		st = amiga.stat(path)
	except amiga.error:
		return 0
	return stat.S_ISREG(st[stat.ST_MODE])


# Are two filenames really pointing to the same file?

def samefile(f1, f2):
	s1 = amiga.stat(f1)
	s2 = amiga.stat(f2)
	return samestat(s1, s2)


# Are two open files really referencing the same file?
# (Not necessarily the same file descriptor!)

def sameopenfile(fp1, fp2):
	s1 = amiga.fstat(fp1)
	s2 = amiga.fstat(fp2)
	return samestat(s1, s2)


# Are two stat buffers (obtained from stat, fstat or lstat)
# describing the same file?

def samestat(s1, s2):
	return s1[stat.ST_INO] == s2[stat.ST_INO] and \
		s1[stat.ST_DEV] == s2[stat.ST_DEV]


# Is a path a mount point? AMIGA: Is it a device name?

def ismount(path):
	drive,rest = splitdrive(path)
	if rest=='':
		return 1
	else:
		return 0


# Directory tree walk.
# For each directory under top (including top itself),
# func(arg, dirname, filenames) is called, where
# dirname is the name of the directory and filenames is the list
# files files (and subdirectories etc.) in the directory.
# The func may modify the filenames list, to implement a filter,
# or to impose a different order of visiting.

def walk(top, func, arg):
	try:
		names = amiga.listdir(top)
	except amiga.error:
		return
	func(arg, top, names)
	for name in names:
		name = join(top, name)
		if isdir(name) and not islink(name):
			walk(name, func, arg)


# Expand paths beginning with '~' or '~user'.
# '~' means $HOME; '~user' means that user's home directory.
# If the path doesn't begin with '~', or if the user or $HOME is unknown,
# the path is returned unchanged (leaving error reporting to whatever
# function is called with the expanded path as argument).
# See also module 'glob' for expansion of *, ? and [...] in pathnames.
# (A function should also be defined to do full *sh-style environment
# variable expansion.)

def expanduser(path):
	if path[:1] <> '~':
		return path
	i, n = 1, len(path)
	while i < n and path[i] <> '/':
		i = i+1
	if i == 1:
		if not amiga.environ.has_key('HOME'):
			return path
		userhome = amiga.environ['HOME']
	else:
		try:
			import pwd
			pwent = pwd.getpwnam(path[1:i])
		except (KeyError,ImportError,SystemError):
			return path			# ~user only works if pwd module works
		userhome = pwent[5]
	if userhome[-1:] == '/': i = i+1
	return userhome + path[i:]


# Expand paths containing shell variable substitutions.
# This expands the forms $variable and ${variable} only.
# Non-existant variables are left unchanged.

_varprog = None

def expandvars(path):
	global _varprog
	if '$' not in path:
		return path
	if not _varprog:
		import regex
		_varprog = regex.compile('$\([a-zA-Z0-9_]+\|{[^}]*}\)')
	i = 0
	while 1:
		i = _varprog.search(path, i)
		if i < 0:
			break
		name = _varprog.group(1)
		j = i + len(_varprog.group(0))
		if name[:1] == '{' and name[-1:] == '}':
			name = name[1:-1]
		if amiga.environ.has_key(name):
			tail = path[j:]
			path = path[:i] + amiga.environ[name]
			i = len(path)
			path = path + tail
		else:
			i = j
	return path


# Normalize a path, e.g. A//B, A/./B and A/foo/../B all become A/B.
# It should be understood that this may change the meaning of the path
# if it contains symbolic links!
# Note: does nothing on Amiga because x/y//z is different than x/y/z.

def normpath(path):
	return path

