print 'This is executed from a separate test script'

try:

	import xyzzy

	print xyzzy.foo()
except ImportError,x:
	print 'importing xyzzy failed',x

result='abc'*9


import sys

print 'sys.path=',sys.path
print 'sys.argv=',sys.argv
print 'sys.exec_prefix=',sys.exec_prefix
print 'sys.executable=',sys.executable
print 'sys.prefix=',sys.prefix
print 'sys.version=',sys.version

