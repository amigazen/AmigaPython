
def dotest():
	print 'This script will now start the Python 1.5 test set.'
	print 'Depending on the speed of your Amiga, this may take several minutes.'
	print
	print 'The report at the end should say something like:'
	print '   32 tests OK.'
	print '   18 tests skipped:  .....'
	print '(If you don\'t start AmiTCP first, test_socket and test_select will fail.)'
	print
	print 'To be 100% sure, run this script TWICE:'
	print 'first, answer \'y\' to the question \'delete .pyc files?\', and the'
	print 'second time, answer \'n\'.'
	print

	# Change to the test directory
	import sys,os
	os.chdir('Python:Lib/Python1.5/test')

	if 'y'==raw_input('Delete .pyc files first? '):
		# Remove all .pyc files
		import glob
		for pyc in glob.glob('*.pyc'):
			os.unlink(pyc)

	print 'Testing...'
	import test.autotest
	print 'Finished.'


if __name__ == '__main__':
	dotest()
