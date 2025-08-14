#!/usr/bin/env python
"""
Amiga Python Build Information Script
Usage: python build_info.py [option]
Options:
  version     - Show version information
  path        - Show module search path
  amiga       - Show Amiga-specific information
  all         - Show all information (default)
"""

import sys

def show_version():
    """Display version information"""
    print("=== Version Information ===")
    print("Python version: %s" % sys.version)
    print("Platform: %s" % sys.platform)
    print("Copyright: %s" % sys.copyright)

def show_path():
    """Display module search path"""
    print("=== Module Search Path ===")
    for i, path in enumerate(sys.path):
        print("  %d: %s" % (i, path))

def show_amiga():
    """Display Amiga-specific information"""
    print("=== Amiga Python Information ===")
    
    # Check for Amiga-specific modules
    amiga_modules = ['amiga']
    for module in amiga_modules:
        try:
            __import__(module)
            print("  %s module: Available" % module)
        except ImportError:
            print("  %s module: Not available" % module)

def show_all():
    """Display all information"""
    show_version()
    print()
    show_path()
    print()
    show_amiga()

def main():
    if len(sys.argv) < 2:
        show_all()
        return
    
    option = sys.argv[1].lower()
    
    if option == 'version':
        show_version()
    elif option == 'path':
        show_path()
    elif option == 'amiga':
        show_amiga()
    elif option == 'all':
        show_all()
    else:
        print("Usage: python build_info.py [option]")
        print("Options: version, path, amiga, all")
        print("Default: all")

if __name__ == '__main__':
    main() 