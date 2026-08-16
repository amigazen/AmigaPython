#!/usr/bin/env python
# AmigaPython 2.7 demo: platform bits via os / amiga.
#
# Run from the release drawer:
#   Python Demo/amiga/amiga_info.py

import os
import sys

def main():
    print 'AmigaPython demo - platform info'
    print 'sys.version :', sys.version.replace('\n', ' ')
    print 'sys.platform:', sys.platform
    print 'sys.prefix  :', sys.prefix
    print 'os.name     :', os.name
    print 'getcwd      :', os.getcwd()

    try:
        import amiga
    except ImportError:
        print 'builtin amiga module not available'
        return

    print 'amiga module:', amiga
    # Safe probes - skip anything that needs optional libraries.
    for name in ('getpid', 'uname', 'getcwd'):
        fn = getattr(amiga, name, None)
        if callable(fn):
            try:
                print 'amiga.%s() ->' % name, fn()
            except (OSError, AttributeError, RuntimeError), err:
                print 'amiga.%s() failed:' % name, err

    if hasattr(amiga, 'to_unix') and hasattr(amiga, 'from_unix'):
        sample = 'Python:lib/site-python'
        try:
            u = amiga.to_unix(sample)
            print 'to_unix(%r) -> %r' % (sample, u)
            print 'from_unix(...) -> %r' % (amiga.from_unix(u),)
        except (OSError, ValueError, TypeError), err:
            print 'path convert failed:', err

    print 'Done.'


if __name__ == '__main__':
    main()
