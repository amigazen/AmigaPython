#!/usr/bin/env python
# RunTest.py - launch Python 2.7 regression tests from this release drawer.
#
# Non-interactive.  Works without ASSIGN Python: when lib/python27.zip
# sits next to this script.

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
	zip1 = os.path.join(here, 'lib', 'python27.zip')
	zip2 = os.path.join(prefix, 'lib', 'python27.zip') if prefix else ''
	for z in (zip1, zip2):
		if z and os.path.isfile(z):
			return 'ZIP:' + z
	return None


def dotest():
	print 'Starting the Python 2.7 regression test set.'
	print 'This may take a long time on classic Amiga hardware.'
	print
	print 'Some tests are skipped on the Amiga.  Without'
	print 'bsdsocket.library, test_socket and test_select will fail.'
	print 'Without usergroup.library, crypt/grp/pwd-style tests may fail.'
	print

	import sys, os
	testdir = _find_testdir()
	if testdir is None:
		print 'Could not find Lib/test or lib/python27.zip next to this script.'
		print 'Keep RunTest.py in the Python release drawer (beside lib/).'
		return

	if testdir.startswith('ZIP:'):
		zpath = testdir[4:]
		print 'Using tests from', zpath
		if zpath not in sys.path:
			sys.path.insert(0, zpath)
		print 'Testing via import test.autotest ...'
		import test.autotest
		print 'Finished.'
		return

	os.chdir(testdir)
	print 'Testing in', testdir, '...'
	import test.autotest
	print 'Finished.'


if __name__ == '__main__':
	dotest()
