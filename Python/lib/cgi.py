#!/usr/local/bin/python

# " <== Emacs font-lock de-bogo-kludgificocity

__version__ = "2.0"


# Imports
# =======

import string
import sys
import os


# Logging support
# ===============

logfile = ""		# Filename to log to, if not empty
logfp = None		# File object to log to, if not None

def initlog(*allargs):
    global logfp, log
    if logfile and not logfp:
	try:
	    logfp = open(logfile, "a")
	except IOError:
	    pass
    if not logfp:
	log = nolog
    else:
	log = dolog
    apply(log, allargs)

def dolog(fmt, *args):
    logfp.write(fmt%args + "\n")

def nolog(*allargs):
    pass

log = initlog		# The current logging function


# Parsing functions
# =================

def parse(fp=None, environ=os.environ, keep_blank_values=None):
    if not fp:
	fp = sys.stdin
    if not environ.has_key('REQUEST_METHOD'):
	environ['REQUEST_METHOD'] = 'GET'	# For testing stand-alone
    if environ['REQUEST_METHOD'] == 'POST':
	ctype, pdict = parse_header(environ['CONTENT_TYPE'])
	if ctype == 'multipart/form-data':
	    return parse_multipart(fp, pdict)
	elif ctype == 'application/x-www-form-urlencoded':
	    clength = string.atoi(environ['CONTENT_LENGTH'])
	    qs = fp.read(clength)
	else:
	    qs = ''			# Unknown content-type
	if environ.has_key('QUERY_STRING'): 
	    if qs: qs = qs + '&'
	    qs = qs + environ['QUERY_STRING']
	elif sys.argv[1:]: 
	    if qs: qs = qs + '&'
	    qs = qs + sys.argv[1]
	environ['QUERY_STRING'] = qs	# XXX Shouldn't, really
    elif environ.has_key('QUERY_STRING'):
	qs = environ['QUERY_STRING']
    else:
	if sys.argv[1:]:
	    qs = sys.argv[1]
	else:
	    qs = ""
	environ['QUERY_STRING'] = qs	# XXX Shouldn't, really
    return parse_qs(qs, keep_blank_values)


def parse_qs(qs, keep_blank_values=None):
    import urllib, regsub
    name_value_pairs = string.splitfields(qs, '&')
    dict = {}
    for name_value in name_value_pairs:
	nv = string.splitfields(name_value, '=')
	if len(nv) != 2:
	    continue
	name = nv[0]
	value = urllib.unquote(regsub.gsub('+', ' ', nv[1]))
        if len(value) or keep_blank_values:
	    if dict.has_key (name):
		dict[name].append(value)
	    else:
		dict[name] = [value]
    return dict


def parse_multipart(fp, pdict):
    import mimetools
    if pdict.has_key('boundary'):
	boundary = pdict['boundary']
    else:
	boundary = ""
    nextpart = "--" + boundary
    lastpart = "--" + boundary + "--"
    partdict = {}
    terminator = ""

    while terminator != lastpart:
	bytes = -1
	data = None
	if terminator:
	    # At start of next part.  Read headers first.
	    headers = mimetools.Message(fp)
	    clength = headers.getheader('content-length')
	    if clength:
		try:
		    bytes = string.atoi(clength)
		except string.atoi_error:
		    pass
	    if bytes > 0:
		data = fp.read(bytes)
	    else:
		data = ""
	# Read lines until end of part.
	lines = []
	while 1:
	    line = fp.readline()
	    if not line:
		terminator = lastpart # End outer loop
		break
	    if line[:2] == "--":
		terminator = string.strip(line)
		if terminator in (nextpart, lastpart):
		    break
	    lines.append(line)
	# Done with part.
	if data is None:
	    continue
	if bytes < 0:
	    if lines:
		# Strip final line terminator
		line = lines[-1]
		if line[-2:] == "\r\n":
		    line = line[:-2]
		elif line[-1:] == "\n":
		    line = line[:-1]
		lines[-1] = line
		data = string.joinfields(lines, "")
	line = headers['content-disposition']
	if not line:
	    continue
	key, params = parse_header(line)
	if key != 'form-data':
	    continue
	if params.has_key('name'):
	    name = params['name']
	else:
	    continue
	if partdict.has_key(name):
	    partdict[name].append(data)
	else:
	    partdict[name] = [data]

    return partdict


def parse_header(line):
    plist = map(string.strip, string.splitfields(line, ';'))
    key = string.lower(plist[0])
    del plist[0]
    pdict = {}
    for p in plist:
	i = string.find(p, '=')
	if i >= 0:
	    name = string.lower(string.strip(p[:i]))
	    value = string.strip(p[i+1:])
	    if len(value) >= 2 and value[0] == value[-1] == '"':
		value = value[1:-1]
	    pdict[name] = value
    return key, pdict


# Classes for field storage
# =========================

class MiniFieldStorage:

    """Like FieldStorage, for use when no file uploads are possible."""

    # Dummy attributes
    filename = None
    list = None
    type = None
    file = None
    type_options = {}
    disposition = None
    disposition_options = {}
    headers = {}

    def __init__(self, name, value):
	"""Constructor from field name and value."""
	from StringIO import StringIO
	self.name = name
	self.value = value
        # self.file = StringIO(value)

    def __repr__(self):
	"""Return printable representation."""
	return "MiniFieldStorage(%s, %s)" % (`self.name`, `self.value`)


class FieldStorage:
    def __init__(self, fp=None, headers=None, outerboundary="",
		 environ=os.environ, keep_blank_values=None):
	method = None
	self.keep_blank_values = keep_blank_values
	if environ.has_key('REQUEST_METHOD'):
	    method = string.upper(environ['REQUEST_METHOD'])
	if not fp and method == 'GET':
	    qs = None
	    if environ.has_key('QUERY_STRING'):
		qs = environ['QUERY_STRING']
	    from StringIO import StringIO
	    fp = StringIO(qs or "")
	    if headers is None:
		headers = {'content-type':
			   "application/x-www-form-urlencoded"}
	if headers is None:
	    headers = {}
	    if environ.has_key('CONTENT_TYPE'):
		headers['content-type'] = environ['CONTENT_TYPE']
	    if environ.has_key('CONTENT_LENGTH'):
		headers['content-length'] = environ['CONTENT_LENGTH']
	self.fp = fp or sys.stdin
	self.headers = headers
	self.outerboundary = outerboundary

	# Process content-disposition header
	cdisp, pdict = "", {}
	if self.headers.has_key('content-disposition'):
	    cdisp, pdict = parse_header(self.headers['content-disposition'])
	self.disposition = cdisp
	self.disposition_options = pdict
	self.name = None
	if pdict.has_key('name'):
	    self.name = pdict['name']
	self.filename = None
	if pdict.has_key('filename'):
	    self.filename = pdict['filename']

	# Process content-type header
	ctype, pdict = "text/plain", {}
	if self.headers.has_key('content-type'):
	    ctype, pdict = parse_header(self.headers['content-type'])
	self.type = ctype
	self.type_options = pdict
	self.innerboundary = ""
	if pdict.has_key('boundary'):
	    self.innerboundary = pdict['boundary']
	clen = -1
	if self.headers.has_key('content-length'):
	    try:
		clen = string.atoi(self.headers['content-length'])
	    except:
		pass
	self.length = clen

	self.list = self.file = None
	self.done = 0
	self.lines = []
	if ctype == 'application/x-www-form-urlencoded':
	    self.read_urlencoded()
	elif ctype[:10] == 'multipart/':
	    self.read_multi()
	else:
	    self.read_single()

    def __repr__(self):
	"""Return a printable representation."""
	return "FieldStorage(%s, %s, %s)" % (
		`self.name`, `self.filename`, `self.value`)

    def __getattr__(self, name):
	if name != 'value':
	    raise AttributeError, name
	if self.file:
	    self.file.seek(0)
	    value = self.file.read()
	    self.file.seek(0)
	elif self.list is not None:
	    value = self.list
	else:
	    value = None
	return value

    def __getitem__(self, key):
	"""Dictionary style indexing."""
	if self.list is None:
	    raise TypeError, "not indexable"
	found = []
	for item in self.list:
	    if item.name == key: found.append(item)
	if not found:
	    raise KeyError, key
	if len(found) == 1:
	    return found[0]
	else:
	    return found

    def keys(self):
	"""Dictionary style keys() method."""
	if self.list is None:
	    raise TypeError, "not indexable"
	keys = []
	for item in self.list:
	    if item.name not in keys: keys.append(item.name)
	return keys

    def has_key(self, key):
	"""Dictionary style has_key() method."""
	if self.list is None:
	    raise TypeError, "not indexable"
	for item in self.list:
	    if item.name == key: return 1
	return 0

    def read_urlencoded(self):
	"""Internal: read data in query string format."""
	qs = self.fp.read(self.length)
        dict = parse_qs(qs, self.keep_blank_values)
	self.list = []
	for key, valuelist in dict.items():
	    for value in valuelist:
		self.list.append(MiniFieldStorage(key, value))
	self.skip_lines()

    def read_multi(self):
	"""Internal: read a part that is itself multipart."""
	import rfc822
	self.list = []
	part = self.__class__(self.fp, {}, self.innerboundary)
	# Throw first part away
	while not part.done:
	    headers = rfc822.Message(self.fp)
	    part = self.__class__(self.fp, headers, self.innerboundary)
	    self.list.append(part)
	self.skip_lines()

    def read_single(self):
	"""Internal: read an atomic part."""
	if self.length >= 0:
	    self.read_binary()
	    self.skip_lines()
	else:
	    self.read_lines()
	self.file.seek(0)

    bufsize = 8*1024		# I/O buffering size for copy to file

    def read_binary(self):
	"""Internal: read binary data."""
	self.file = self.make_file('b')
	todo = self.length
	if todo >= 0:
	    while todo > 0:
		data = self.fp.read(min(todo, self.bufsize))
		if not data:
		    self.done = -1
		    break
		self.file.write(data)
		todo = todo - len(data)

    def read_lines(self):
	"""Internal: read lines until EOF or outerboundary."""
	self.file = self.make_file('')
	if self.outerboundary:
	    self.read_lines_to_outerboundary()
	else:
	    self.read_lines_to_eof()

    def read_lines_to_eof(self):
	"""Internal: read lines until EOF."""
	while 1:
	    line = self.fp.readline()
	    if not line:
		self.done = -1
		break
	    self.lines.append(line)
	    self.file.write(line)

    def read_lines_to_outerboundary(self):
	"""Internal: read lines until outerboundary."""
	next = "--" + self.outerboundary
	last = next + "--"
	delim = ""
	while 1:
	    line = self.fp.readline()
	    if not line:
		self.done = -1
		break
	    self.lines.append(line)
	    if line[:2] == "--":
		strippedline = string.strip(line)
		if strippedline == next:
		    break
		if strippedline == last:
		    self.done = 1
		    break
	    odelim = delim
	    if line[-2:] == "\r\n":
		delim = "\r\n"
		line = line[:-2]
	    elif line[-1] == "\n":
		delim = "\n"
		line = line[:-1]
	    else:
		delim = ""
	    self.file.write(odelim + line)

    def skip_lines(self):
	"""Internal: skip lines until outer boundary if defined."""
	if not self.outerboundary or self.done:
	    return
	next = "--" + self.outerboundary
	last = next + "--"
	while 1:
	    line = self.fp.readline()
	    if not line:
		self.done = -1
		break
	    self.lines.append(line)
	    if line[:2] == "--":
		strippedline = string.strip(line)
		if strippedline == next:
		    break
		if strippedline == last:
		    self.done = 1
		    break

    def make_file(self, binary):
	import tempfile
	tfn = tempfile.mktemp()
	f = open(tfn, "w%s+" % binary)
	os.unlink(tfn)
	return f


# Backwards Compatibility Classes
# ===============================

class FormContentDict:
    def __init__(self, environ=os.environ):
        self.dict = parse(environ=environ)
	self.query_string = environ['QUERY_STRING']
    def __getitem__(self,key):
	return self.dict[key]
    def keys(self):
	return self.dict.keys()
    def has_key(self, key):
	return self.dict.has_key(key)
    def values(self):
	return self.dict.values()
    def items(self):
	return self.dict.items() 
    def __len__( self ):
	return len(self.dict)


class SvFormContentDict(FormContentDict):
    def __getitem__(self, key):
	if len(self.dict[key]) > 1: 
	    raise IndexError, 'expecting a single value' 
	return self.dict[key][0]
    def getlist(self, key):
	return self.dict[key]
    def values(self):
	lis = []
	for each in self.dict.values(): 
	    if len( each ) == 1 : 
		lis.append(each[0])
	    else: lis.append(each)
	return lis
    def items(self):
	lis = []
	for key,value in self.dict.items():
	    if len(value) == 1 :
		lis.append((key, value[0]))
	    else:	lis.append((key, value))
	return lis


class InterpFormContentDict(SvFormContentDict):
    """This class is present for backwards compatibility only.""" 
    def __getitem__( self, key ):
	v = SvFormContentDict.__getitem__( self, key )
	if v[0] in string.digits+'+-.' : 
	    try:  return  string.atoi( v ) 
	    except ValueError:
		try:	return string.atof( v )
		except ValueError: pass
	return string.strip(v)
    def values( self ):
	lis = [] 
	for key in self.keys():
	    try:
		lis.append( self[key] )
	    except IndexError:
		lis.append( self.dict[key] )
	return lis
    def items( self ):
	lis = [] 
	for key in self.keys():
	    try:
		lis.append( (key, self[key]) )
	    except IndexError:
		lis.append( (key, self.dict[key]) )
	return lis


class FormContent(FormContentDict):
    """This class is present for backwards compatibility only.""" 
    def values(self, key):
	if self.dict.has_key(key) :return self.dict[key]
	else: return None
    def indexed_value(self, key, location):
	if self.dict.has_key(key):
	    if len (self.dict[key]) > location:
		return self.dict[key][location]
	    else: return None
	else: return None
    def value(self, key):
	if self.dict.has_key(key): return self.dict[key][0]
	else: return None
    def length(self, key):
	return len(self.dict[key])
    def stripped(self, key):
	if self.dict.has_key(key): return string.strip(self.dict[key][0])
	else: return None
    def pars(self):
	return self.dict


# Test/debug code
# ===============

def test(environ=os.environ):
    """Robust test CGI script, usable as main program.

    Write minimal HTTP headers and dump all information provided to
    the script in HTML form.

    """
    import traceback
    print "Content-type: text/html"
    print
    sys.stderr = sys.stdout
    try:
	form = FieldStorage()	# Replace with other classes to test those
	print_form(form)
        print_environ(environ)
	print_directory()
	print_arguments()
	print_environ_usage()
	def f():
	    exec "testing print_exception() -- <I>italics?</I>"
	def g(f=f):
	    f()
	print "<H3>What follows is a test, not an actual exception:</H3>"
	g()
    except:
	print_exception()

def print_exception(type=None, value=None, tb=None, limit=None):
    if type is None:
	type, value, tb = sys.exc_type, sys.exc_value, sys.exc_traceback
    import traceback
    print
    print "<H3>Traceback (innermost last):</H3>"
    list = traceback.format_tb(tb, limit) + \
	   traceback.format_exception_only(type, value)
    print "<PRE>%s<B>%s</B></PRE>" % (
	escape(string.join(list[:-1], "")),
	escape(list[-1]),
	)

def print_environ(environ=os.environ):
    """Dump the shell environment as HTML."""
    keys = environ.keys()
    keys.sort()
    print
    print "<H3>Shell Environment:</H3>"
    print "<DL>"
    for key in keys:
	print "<DT>", escape(key), "<DD>", escape(environ[key])
    print "</DL>" 
    print

def print_form(form):
    """Dump the contents of a form as HTML."""
    keys = form.keys()
    keys.sort()
    print
    print "<H3>Form Contents:</H3>"
    print "<DL>"
    for key in keys:
	print "<DT>" + escape(key) + ":",
	value = form[key]
	print "<i>" + escape(`type(value)`) + "</i>"
	print "<DD>" + escape(`value`)
    print "</DL>"
    print

def print_directory():
    """Dump the current directory as HTML."""
    print
    print "<H3>Current Working Directory:</H3>"
    try:
	pwd = os.getcwd()
    except os.error, msg:
	print "os.error:", escape(str(msg))
    else:
	print escape(pwd)
    print

def print_arguments():
    print
    print "<H3>Command Line Arguments:</H3>"
    print
    print sys.argv
    print

def print_environ_usage():
    """Dump a list of environment variables used by CGI as HTML."""
    print """
<H3>These environment variables could have been set:</H3>
<UL>
<LI>AUTH_TYPE
<LI>CONTENT_LENGTH
<LI>CONTENT_TYPE
<LI>DATE_GMT
<LI>DATE_LOCAL
<LI>DOCUMENT_NAME
<LI>DOCUMENT_ROOT
<LI>DOCUMENT_URI
<LI>GATEWAY_INTERFACE
<LI>LAST_MODIFIED
<LI>PATH
<LI>PATH_INFO
<LI>PATH_TRANSLATED
<LI>QUERY_STRING
<LI>REMOTE_ADDR
<LI>REMOTE_HOST
<LI>REMOTE_IDENT
<LI>REMOTE_USER
<LI>REQUEST_METHOD
<LI>SCRIPT_NAME
<LI>SERVER_NAME
<LI>SERVER_PORT
<LI>SERVER_PROTOCOL
<LI>SERVER_ROOT
<LI>SERVER_SOFTWARE
</UL>
In addition, HTTP headers sent by the server may be passed in the
environment as well.  Here are some common variable names:
<UL>
<LI>HTTP_ACCEPT
<LI>HTTP_CONNECTION
<LI>HTTP_HOST
<LI>HTTP_PRAGMA
<LI>HTTP_REFERER
<LI>HTTP_USER_AGENT
</UL>
"""


# Utilities
# =========

def escape(s):
    """Replace special characters '&', '<' and '>' by SGML entities."""
    import regsub
    s = regsub.gsub("&", "&amp;", s)	# Must be done first!
    s = regsub.gsub("<", "&lt;", s)
    s = regsub.gsub(">", "&gt;", s)
    return s


# Invoke mainline
# ===============

# Call test() when this file is run as a script (not imported as a module)
if __name__ == '__main__': 
    test()
