"""
Path operations common to more than one OS
Do not use directly.  The OS specific modules import the appropriate
functions from this module themselves.
"""
import os
import stat

__all__ = ['commonprefix', 'exists', 'getatime', 'getctime', 'getmtime',
           'getsize', 'isdir', 'isfile']


try:
    _unicode = unicode
except NameError:
    # If Python is built without Unicode support, the unicode type
    # will not exist. Fake one.
    class _unicode(object):
        pass

# Does a path exist?
# This is false for dangling symbolic links on systems that support them.
def exists(path):
    """Test whether a path exists.  Returns False for broken symbolic links"""
    try:
        os.stat(path)
    except os.error:
        return False
    return True


# This follows symbolic links, so both islink() and isdir() can be true
# for the same path on systems that support symlinks
def isfile(path):
    """Test whether a path is a regular file"""
    try:
        st = os.stat(path)
    except os.error:
        return False
    try:
        mode = st.st_mode
    except AttributeError:
        mode = st[stat.ST_MODE]
    return stat.S_ISREG(mode)


# Is a path a directory?
# This follows symbolic links, so both islink() and isdir()
# can be true for the same path on systems that support symlinks
def isdir(s):
    """Return true if the pathname refers to an existing directory."""
    try:
        st = os.stat(s)
    except os.error:
        return False
    try:
        mode = st.st_mode
    except AttributeError:
        mode = st[stat.ST_MODE]
    return stat.S_ISDIR(mode)


def getsize(filename):
    """Return the size of a file, reported by os.stat()."""
    st = os.stat(filename)
    try:
        return st.st_size
    except AttributeError:
        return st[stat.ST_SIZE]


def getmtime(filename):
    """Return the last modification time of a file, reported by os.stat()."""
    st = os.stat(filename)
    try:
        return st.st_mtime
    except AttributeError:
        return st[stat.ST_MTIME]


def getatime(filename):
    """Return the last access time of a file, reported by os.stat()."""
    st = os.stat(filename)
    try:
        return st.st_atime
    except AttributeError:
        return st[stat.ST_ATIME]


def getctime(filename):
    """Return the metadata change time of a file, reported by os.stat()."""
    st = os.stat(filename)
    try:
        return st.st_ctime
    except AttributeError:
        return st[stat.ST_CTIME]


# Return the longest prefix of all list elements.
def commonprefix(m):
    "Given a list of pathnames, returns the longest common leading component"
    if not m: return ''
    s1 = min(m)
    s2 = max(m)
    for i, c in enumerate(s1):
        if c != s2[i]:
            return s1[:i]
    return s1

# Split a path in root and extension.
# The extension is everything starting at the last dot in the last
# pathname component; the root is everything before that.
# It is always true that root + ext == p.

# Generic implementation of splitext, to be parametrized with
# the separators
def _splitext(p, sep, altsep, extsep):
    """Split the extension from a pathname.

    Extension is everything from the last dot to the end, ignoring
    leading dots.  Returns "(root, ext)"; ext may be empty."""

    sepIndex = p.rfind(sep)
    if altsep:
        altsepIndex = p.rfind(altsep)
        sepIndex = max(sepIndex, altsepIndex)

    dotIndex = p.rfind(extsep)
    if dotIndex > sepIndex:
        # skip all leading dots
        filenameIndex = sepIndex + 1
        while filenameIndex < dotIndex:
            if p[filenameIndex] != extsep:
                return p[:dotIndex], p[dotIndex:]
            filenameIndex += 1

    return p, ''
