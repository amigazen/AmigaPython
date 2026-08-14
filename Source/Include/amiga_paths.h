/*
 * amiga_paths.h - Amiga path helpers for Python 2.7.18
 *
 * vbcc PosixLib fopen/stat accept Amiga volume:path names natively
 * (they convert internally).  Do not rewrite them as /volume/path --
 * that form breaks directory detection (NullImporter) and imports.
 *
 * Py_AmigaToPosixPath() copies the Amiga path through unchanged so
 * call sites share one helper if PosixLib behaviour ever changes.
 */

#ifndef AMIGA_PATHS_H
#define AMIGA_PATHS_H

#ifdef _AMIGA

#include <stddef.h>

/* Copy amiga_path into dest.  dest_len includes the trailing NUL. */
void Py_AmigaToPosixPath(char *dest, size_t dest_len, const char *amiga_path);

#endif /* _AMIGA */

#endif /* AMIGA_PATHS_H */
