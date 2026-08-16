#!/usr/bin/env python
"""Count XML elements/attributes via xml.sax (AmigaPython 2.7 / Expat).

Usage: Python elem_count.py file.xml
"""
import sys

from xml.sax import make_parser, handler

class FancyCounter(handler.ContentHandler):

    def __init__(self):
        self._elems = 0
        self._attrs = 0
        self._elem_types = {}
        self._attr_types = {}

    def startElement(self, name, attrs):
        self._elems = self._elems + 1
        self._attrs = self._attrs + len(attrs)
        self._elem_types[name] = self._elem_types.get(name, 0) + 1

        for aname in attrs.keys():
            self._attr_types[aname] = self._attr_types.get(aname, 0) + 1

    def endDocument(self):
        print "There were", self._elems, "elements."
        print "There were", self._attrs, "attributes."

        print "---ELEMENT TYPES"
        for pair in self._elem_types.items():
            print "%20s %d" % pair

        print "---ATTRIBUTE TYPES"
        for pair in self._attr_types.items():
            print "%20s %d" % pair


def main(argv):
    if len(argv) < 2:
        print >> sys.stderr, 'Usage: %s file.xml' % argv[0]
        return 2
    parser = make_parser()
    parser.setContentHandler(FancyCounter())
    parser.parse(argv[1])
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
