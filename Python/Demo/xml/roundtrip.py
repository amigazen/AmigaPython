#!/usr/bin/env python
"""SAX XML roundtrip demo (AmigaPython 2.7 / Expat).

Usage: Python roundtrip.py file.xml
"""

import sys

from xml.sax import saxutils, handler, make_parser

class ContentGenerator(handler.ContentHandler):

    def __init__(self, out=sys.stdout):
        handler.ContentHandler.__init__(self)
        self._out = out

    def startDocument(self):
        self._out.write('<?xml version="1.0" encoding="iso-8859-1"?>\n')

    def startElement(self, name, attrs):
        self._out.write('<' + name)
        for (aname, value) in attrs.items():
            self._out.write(' %s="%s"' % (aname, saxutils.escape(value)))
        self._out.write('>')

    def endElement(self, name):
        self._out.write('</%s>' % name)

    def characters(self, content):
        self._out.write(saxutils.escape(content))

    def ignorableWhitespace(self, content):
        self._out.write(content)

    def processingInstruction(self, target, data):
        self._out.write('<?%s %s?>' % (target, data))


def main(argv):
    if len(argv) < 2:
        print >> sys.stderr, 'Usage: %s file.xml' % argv[0]
        return 2
    parser = make_parser()
    parser.setContentHandler(ContentGenerator())
    parser.parse(argv[1])
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
