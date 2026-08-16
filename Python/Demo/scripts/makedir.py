#! /usr/bin/env python
# Create drawers recursively (AmigaPython 2.7).
# Usage: Python makedir.py RAM:T/One/Two/Three

import sys
import os

def main(argv):
	if len(argv) < 2:
		print >> sys.stderr, 'Usage: %s path [path...]' % argv[0]
		return 2
	for p in argv[1:]:
		if not os.path.isdir(p):
			os.makedirs(p)
			print 'created', p
		else:
			print 'exists ', p
	return 0

if __name__ == '__main__':
	sys.exit(main(sys.argv))
