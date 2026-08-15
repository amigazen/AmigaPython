
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

	# Change to the test directory under the versioned library
	import sys, os
	candidates = [
		'Python:Lib/test',
		'Python:Lib/python2.7/test',
		'Python:lib/test',
		'Python:lib/python2.7/test',
	]
	testdir = None
	for path in candidates:
		if os.path.isdir(path):
			testdir = path
			break
	if testdir is None:
		# Fall back relative to this script / sys.prefix
		here = os.path.dirname(sys.argv[0])
		for rel in ('Lib/test', 'lib/test', 'Lib/python2.7/test'):
			path = os.path.join(here, rel)
			if os.path.isdir(path):
				testdir = path
				break
	if testdir is None:
		print 'Could not find Lib/test.  Set ASSIGN Python: and retry.'
		return
	os.chdir(testdir)

	if 'y' == raw_input('Delete .pyc files first? '):
		import glob
		for pyc in glob.glob('*.pyc'):
			os.unlink(pyc)

	print 'Testing in', testdir, '...'
	import test.autotest
	print 'Finished.'


if __name__ == '__main__':
	dotest()
