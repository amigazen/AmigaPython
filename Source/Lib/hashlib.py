# $Id$
#
#  Copyright (C) 2005   Gregory P. Smith (greg@krypto.org)
#  Licensed to PSF under a Contributor Agreement.
#
# Amiga notes:
# - ASCII only (Amiga Python 2.7 has no default UTF-8 source encoding).
# - _hashlib is Modules/_hashcrc.c wrapping crc.library 2.x
#   (MD5, SHA-1, SHA-256). Do not probe missing sha2 names at import time.

__doc__ = """hashlib module - A common interface to many hash functions.

new(name, string='') - returns a new hash object implementing the
    given hash function; initializing the hash
    using the given string data.

Named constructor functions are also available:

md5(), sha1(), sha224(), sha256(), sha384(), and sha512()

On Amiga with crc.library, md5/sha1/sha256 are provided by _hashlib.
sha224/sha384/sha512 are not available from that library.

See algorithms_guaranteed and algorithms_available for what this
build actually exposes.

NOTE: adler32 / crc32 live in the zlib module.
"""

import sys as _sys

# Amiga: only claim what crc.library provides so import does not walk
# missing openssl_sha224/etc constructors (each probe used to CRCNew).
if getattr(_sys, 'platform', '') == 'amiga':
    __always_supported = ('md5', 'sha1', 'sha256')
else:
    __always_supported = ('md5', 'sha1', 'sha224', 'sha256', 'sha384', 'sha512')

algorithms_guaranteed = set(__always_supported)
algorithms_available = set(__always_supported)

algorithms = __always_supported

__all__ = __always_supported + ('new', 'algorithms_guaranteed',
                                'algorithms_available', 'algorithms',
                                'pbkdf2_hmac')


def __get_builtin_constructor(name):
    # Fallback when _hashlib is missing: Amiga inittab uses "md5" / "sha".
    try:
        if name in ('SHA1', 'sha1'):
            try:
                import _sha
            except ImportError:
                import sha as _sha
            return _sha.new
        elif name in ('MD5', 'md5'):
            try:
                import _md5
            except ImportError:
                import md5 as _md5
            return _md5.new
        elif name in ('SHA256', 'sha256', 'SHA224', 'sha224'):
            import _sha256
            bs = name[3:]
            if bs == '256':
                return _sha256.sha256
            elif bs == '224':
                return _sha256.sha224
        elif name in ('SHA512', 'sha512', 'SHA384', 'sha384'):
            import _sha512
            bs = name[3:]
            if bs == '512':
                return _sha512.sha512
            elif bs == '384':
                return _sha512.sha384
    except ImportError:
        pass

    raise ValueError('unsupported hash type ' + name)


def __get_openssl_constructor(name):
    try:
        f = getattr(_hashlib, 'openssl_' + name)
        # Stock CPython calls f() here to see if OpenSSL really has the
        # hash. On Amiga that would CRCNew during "import hashlib"; just
        # bind the constructor instead.
        if getattr(_sys, 'platform', '') != 'amiga':
            f()
        return f
    except (AttributeError, ValueError):
        return __get_builtin_constructor(name)


def __py_new(name, string=''):
    return __get_builtin_constructor(name)(string)


def __hash_new(name, string=''):
    try:
        return _hashlib.new(name, string)
    except ValueError:
        return __get_builtin_constructor(name)(string)


try:
    import _hashlib
    new = __hash_new
    __get_hash = __get_openssl_constructor
    algorithms_available = algorithms_available.union(
        _hashlib.openssl_md_meth_names)
except ImportError:
    new = __py_new
    __get_hash = __get_builtin_constructor

for __func_name in __always_supported:
    try:
        globals()[__func_name] = __get_hash(__func_name)
    except ValueError:
        # Do not import logging here -- that has crashed this port during
        # "import hashlib" while random/tests were loading.
        algorithms_available.discard(__func_name)

algorithms_guaranteed = set([n for n in __always_supported
                             if n in algorithms_available])
algorithms = tuple(n for n in __always_supported if n in algorithms_available)

try:
    from _hashlib import pbkdf2_hmac
except ImportError:
    # Build translate tables lazily -- avoids 512-char work at import time.
    _trans_5C = None
    _trans_36 = None

    def pbkdf2_hmac(hash_name, password, salt, iterations, dklen=None):
        """Password based key derivation function 2 (PKCS #5 v2.0)"""
        global _trans_5C, _trans_36
        import binascii
        import struct

        if not isinstance(hash_name, str):
            raise TypeError(hash_name)

        if not isinstance(password, (bytes, bytearray)):
            password = bytes(buffer(password))
        if not isinstance(salt, (bytes, bytearray)):
            salt = bytes(buffer(salt))

        if _trans_5C is None:
            _trans_5C = b"".join(chr(x ^ 0x5C) for x in range(256))
            _trans_36 = b"".join(chr(x ^ 0x36) for x in range(256))

        inner = new(hash_name)
        outer = new(hash_name)
        blocksize = getattr(inner, 'block_size', 64)
        if len(password) > blocksize:
            password = new(hash_name, password).digest()
        password = password + b'\x00' * (blocksize - len(password))
        inner.update(password.translate(_trans_36))
        outer.update(password.translate(_trans_5C))

        def prf(msg, inner=inner, outer=outer):
            icpy = inner.copy()
            ocpy = outer.copy()
            icpy.update(msg)
            ocpy.update(icpy.digest())
            return ocpy.digest()

        if iterations < 1:
            raise ValueError(iterations)
        if dklen is None:
            dklen = outer.digest_size
        if dklen < 1:
            raise ValueError(dklen)

        hex_format_string = "%%0%ix" % (new(hash_name).digest_size * 2)

        dkey = b''
        loop = 1
        while len(dkey) < dklen:
            prev = prf(salt + struct.pack(b'>I', loop))
            rkey = int(binascii.hexlify(prev), 16)
            for i in xrange(iterations - 1):
                prev = prf(prev)
                rkey ^= int(binascii.hexlify(prev), 16)
            loop += 1
            dkey += binascii.unhexlify(hex_format_string % rkey)

        return dkey[:dklen]

del __always_supported, __func_name, __get_hash
del __py_new, __hash_new, __get_openssl_constructor
del _sys
