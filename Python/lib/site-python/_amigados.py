"""
Python-level dos.library helpers installed onto builtin amiga.

Loaded by site.py on Amiga (not from initamiga - that races with os.py).
Do not import this module directly; use import amiga.
"""

import string
import os
import time
import errno
import sys

amiga = sys.modules['amiga']

# Modes for SetMode / SetConsoleMode
MODE_CON = 0
MODE_RAW = 1

# Bits that signal you that a user has issued a break
SIGBREAKF_CTRL_C = 1 << 12
SIGBREAKF_CTRL_D = 1 << 13
SIGBREAKF_CTRL_E = 1 << 14
SIGBREAKF_CTRL_F = 1 << 15

# FILE PROTECTION BITS
FIBF_SCRIPT = 1 << 6
FIBF_PURE = 1 << 5
FIBF_ARCHIVE = 1 << 4
FIBF_READ = 1 << 3
FIBF_WRITE = 1 << 2
FIBF_EXECUTE = 1 << 1
FIBF_DELETE = 1 << 0
FIBF_OTR_READ = 1 << 15
FIBF_OTR_WRITE = 1 << 14
FIBF_OTR_EXECUTE = 1 << 13
FIBF_OTR_DELETE = 1 << 12
FIBF_GRP_READ = 1 << 11
FIBF_GRP_WRITE = 1 << 10
FIBF_GRP_EXECUTE = 1 << 9
FIBF_GRP_DELETE = 1 << 8

# FLAGS FOR DateToStr AND StrToDate
DTF_SUBST = 1 << 0
DTB_FUTURE = 1 << 1

# FORMATS FOR DateToStr AND StrToDate
FORMAT_DOS = 0
FORMAT_INT = 1
FORMAT_USA = 2
FORMAT_CDN = 3

# EXAMINE struct members (FileInfoBlock)
fib_FileName = 0
fib_Size = 1
fib_DirEntryType = 2
fib_Protection = 3
fib_DiskKey = 4
fib_NumBlocks = 5
fib_Date = 6
fib_Comment = 7
fib_OwnerUID = 8
fib_OwnerGID = 9

# INFO struct members (InfoData)
id_NumSoftErrors = 0
id_UnitNumber = 1
id_DiskState = 2
id_NumBlocks = 3
id_NumBlocksUsed = 4
id_BytesPerBlock = 5
id_DiskType = 6
id_InUse = 7

ID_WRITE_PROTECTED = 80
ID_VALIDATING = 81
ID_VALIDATED = 82


class ArgParser:
	"""
	Argument string parser for dos.library/ReadArgs() templates.
	"""
	def __init__(self, template):
		self.new(template)
	def new(self, template):
		self.template = template
		self.reset()
	def reset(self):
		self.defaults, self.types = self.parsetempl(self.template)
	def parse(self, args):
		result = amiga.ReadArgs(self.template, args, self.types)
		for k in result.keys():
			if not result[k]:
				dflt = self.defaults[k]
				if type(dflt) != type([]):
					result[k] = dflt
				else:
					result[k] = dflt[:]
		return result

	def parsetempl(self, templ):
		if not templ:
			return ({}, ())

		for c in templ:
			if not c in '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.,=/ ':
				raise ValueError, 'invalid char in template (' + c + ')'

		templ = map(lambda x: string.split(x, '/'), string.split(templ, ','))
		types = []; defdict = {}; lists = 0; keywdic = {}
		defaults = {'S': 0, 'T': 0, 'N': None, 'X': None, 'A': [], 'I': []}
		for a in templ:
			if a[0] == '':
				raise ValueError, 'missing keyword in template'

			type = None; modifiers = ''
			for s in map(string.upper, a[1:]):
				if not s:
					raise ValueError, 'invalid template'
				elif s == 'T':
					raise SystemError, '/T not (yet) supported, sorry'
				elif s in 'SNT':
					if type:
						raise ValueError, 'invalid switch combination at ' + a[0]
					type = s
				elif s in 'KAMF':
					if s in modifiers or \
					   (s == 'M' and 'F' in modifiers) or \
					   (s == 'F' and 'M' in modifiers):
						raise ValueError, 'invalid switch combination at ' + a[0]
					modifiers = modifiers + s
				else:
					raise ValueError, 'unknown switch /' + s

			if not type:
				type = 'X'

			if 'M' in modifiers:
				lists = lists + 1
				if lists > 1:
					raise ValueError, 'multiple /M switches'
				if type == 'X': type = 'A'
				elif type == 'N': type = 'I'
				else:
					raise ValueError, 'wrong /M combination'

			# One tuple per keyword: (name, type-letter).
			types.append((a[0], type))
			try:
				keywdic[a[0]] = keywdic[a[0]] + 1
			except KeyError:
				keywdic[a[0]] = 1
			if not 'A' in modifiers:
				defdict[a[0]] = defaults[type]

		keywdic = keywdic.values()
		if keywdic.count(1) != len(keywdic):
			raise ValueError, 'clashing keywords'

		return (defdict, tuple(types))


def touch(file, tme=None):
	if not tme:
		tme = time.time()
	tme = amiga.time2DS(tme)
	try:
		os.stat(file)
	except os.error, x:
		if (x[0] == errno.ENOENT):
			open(file, 'w')
		else:
			raise os.error, x
	amiga.SetFileDate(file, tme)


def AddBuffers(drive, buffers):
	return os.system('c:addbuffers >NIL: "%s" %d' % (drive, buffers))

def AssignAdd(name, target):
	return os.system('c:assign >NIL: "%s" "%s" ADD' % (name, target))

def AssignRemove(name):
	return os.system('c:assign >NIL: "%s" REMOVE' % name)


# Install public names onto the builtin amiga module.
_EXPORT = (
	'MODE_CON', 'MODE_RAW',
	'SIGBREAKF_CTRL_C', 'SIGBREAKF_CTRL_D', 'SIGBREAKF_CTRL_E', 'SIGBREAKF_CTRL_F',
	'FIBF_SCRIPT', 'FIBF_PURE', 'FIBF_ARCHIVE', 'FIBF_READ', 'FIBF_WRITE',
	'FIBF_EXECUTE', 'FIBF_DELETE',
	'FIBF_OTR_READ', 'FIBF_OTR_WRITE', 'FIBF_OTR_EXECUTE', 'FIBF_OTR_DELETE',
	'FIBF_GRP_READ', 'FIBF_GRP_WRITE', 'FIBF_GRP_EXECUTE', 'FIBF_GRP_DELETE',
	'DTF_SUBST', 'DTB_FUTURE',
	'FORMAT_DOS', 'FORMAT_INT', 'FORMAT_USA', 'FORMAT_CDN',
	'fib_FileName', 'fib_Size', 'fib_DirEntryType', 'fib_Protection',
	'fib_DiskKey', 'fib_NumBlocks', 'fib_Date', 'fib_Comment',
	'fib_OwnerUID', 'fib_OwnerGID',
	'id_NumSoftErrors', 'id_UnitNumber', 'id_DiskState', 'id_NumBlocks',
	'id_NumBlocksUsed', 'id_BytesPerBlock', 'id_DiskType', 'id_InUse',
	'ID_WRITE_PROTECTED', 'ID_VALIDATING', 'ID_VALIDATED',
	'ArgParser', 'touch', 'AddBuffers', 'AssignAdd', 'AssignRemove',
)

# Compat alias used by older ARexx host code (Dos.error).
amiga.error_dos = amiga.doserror

g = globals()
for _name in _EXPORT:
	setattr(amiga, _name, g[_name])
