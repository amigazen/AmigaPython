
print 'This script will now start the Python 1.4 test set.'
print 'Depending on the speed of your Amiga, this may take several minutes.'
print
print 'You should see "All tests OK.", if all tests are passed correctly.'
print 'To be 100% sure, run this script TWICE:'
print 'first, answer \'y\' to the question \'delete .pyc files?\', and the'
print 'second time, answer \'n\'.'
print

# Change to the test directory
import sys,os
os.chdir('Python:Lib/test')
sys.path.insert(0,os.curdir)

if 'y'==raw_input('Delete .pyc files first? '):
	# Remove all .pyc files
	import glob
	for pyc in glob.glob('*.pyc'):
		os.unlink(pyc)

print 'Testing...'
import autotest
print 'Finished.'
