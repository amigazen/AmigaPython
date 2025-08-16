/*
 * amiga_paths.h - Amiga path conversion macros for Python 2.7.18
 * 
 * This header provides macros that override standard POSIX functions
 * to handle Amiga-style paths by converting them to Unix-style paths
 * before calling the actual PosixLib functions.
 */

#ifndef AMIGA_PATHS_H
#define AMIGA_PATHS_H

#ifdef _AMIGA

/* Override standard functions with Amiga path-aware versions */
#define stat(path, buf) amiga_stat(path, buf)
#define lstat(path, buf) amiga_lstat(path, buf)
#define access(path, mode) amiga_access(path, mode)
#define open(path, flags, ...) amiga_open(path, flags, ##__VA_ARGS__)

/* Function declarations */
int amiga_stat(const char* path, struct stat* buf);
int amiga_lstat(const char* path, struct stat* buf);
int amiga_access(const char* path, int mode);
int amiga_open(const char* path, int flags, ...);

#endif /* _AMIGA */

#endif /* AMIGA_PATHS_H */ 