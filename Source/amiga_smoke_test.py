#!/usr/bin/env python
# Compatibility entry point -- runs the full Amiga port suite.
# Prefer: python27 AmigaTests/run.py

from __future__ import print_function

import os
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
if _HERE not in sys.path:
    sys.path.insert(0, _HERE)

from AmigaTests.run import main

if __name__ == "__main__":
    sys.exit(main())
