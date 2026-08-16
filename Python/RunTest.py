#!/usr/bin/env python
# RunTest.py - launch Python 2.7 regression tests from this release drawer.
#
# Works without ASSIGN Python: when lib/python27.zip sits next to this
# script.  From Workbench, pauses at the end so the CON: window stays
# readable (no WAIT on the console path).

def _script_dir():
	import os, sys
	if sys.argv and sys.argv[0]:
		d = os.path.dirname(sys.argv[0])
		if d:
			return d
	return os.getcwd()


def _find_testdir():
	import os, sys
	here = _script_dir()
	prefix = getattr(sys, 'prefix', '') or ''
	candidates = [
		os.path.join(here, 'Lib', 'test'),
		os.path.join(here, 'lib', 'test'),
		os.path.join(here, 'Lib', 'python2.7', 'test'),
		os.path.join(here, 'lib', 'python2.7', 'test'),
		'Python:Lib/test',
		'Python:Lib/python2.7/test',
		'Python:lib/test',
		'Python:lib/python2.7/test',
	]
	if prefix:
		candidates[0:0] = [
			os.path.join(prefix, 'Lib', 'test'),
			os.path.join(prefix, 'lib', 'test'),
			os.path.join(prefix, 'Lib', 'python2.7', 'test'),
			os.path.join(prefix, 'lib', 'python2.7', 'test'),
		]
	for path in candidates:
		if os.path.isdir(path):
			return path
	# Zipimport: tests live inside python27.zip as Lib/test or test.
	zip1 = os.path.join(here, 'lib', 'python27.zip')
	zip2 = os.path.join(prefix, 'lib', 'python27.zip') if prefix else ''
	for z in (zip1, zip2):
		if z and os.path.isfile(z):
			# Prefer running via import test.autotest with zip on path.
			return 'ZIP:' + z
	return None


def _pause_if_interactive():
	import sys
	try:
		if not hasattr(sys.stdin, 'isatty') or not sys.stdin.isatty():
			return
		raw_input('\nPress Return to close...')
	except (EOFError, KeyboardInterrupt):
		pass


def dotest():
	print 'This script will now start the Python 2.7 regression test set.'
	print 'Depending on the speed of your Amiga, this may take a long time.'
	print
	print 'Some tests are skipped on the Amiga.  If you do not have'
	print 'bsdsocket.library available, test_socket and test_select will fail.'
	print 'Without usergroup.library, crypt/grp/pwd-style tests may fail.'
	print
	print 'To be thorough, run this script TWICE:'
	print "first, answer 'y' to delete .pyc files, then again answering 'n'."
	print

	import sys, os
	testdir = _find_testdir()
	if testdir is None:
		print 'Could not find Lib/test or lib/python27.zip next to this script.'
		print 'Keep RunTest.py in the Python release drawer (beside lib/).'
		_pause_if_interactive()
		return

	if testdir.startswith('ZIP:'):
		zpath = testdir[4:]
		print 'Using tests from', zpath
		# Ensure zip is importable; getpath usually already added it.
		if zpath not in sys.path:
			sys.path.insert(0, zpath)
		# Delete .pyc only applies to unpacked trees.
		ans = raw_input('Delete .pyc files first? (only if you have an unpacked Lib/test) [n] ')
		if ans[:1].lower() == 'y':
			print '(No unpacked test dir; skipping .pyc delete.)'
		print 'Testing via import test.autotest ...'
		import test.autotest
		print 'Finished.'
		_pause_if_interactive()
		return

	os.chdir(testdir)
	if 'y' == raw_input('Delete .pyc files first? '):
		import glob
		for pyc in glob.glob('*.pyc'):
			os.unlink(pyc)

	print 'Testing in', testdir, '...'
	import test.autotest
	print 'Finished.'
	_pause_if_interactive()


if __name__ == '__main__':
	dotest()
