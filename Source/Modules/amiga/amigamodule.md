# Amiga Module Enhancement Plan for AmigaOS4 Compatibility

## Overview

This document outlines the changes required to `amigamodule.c` to achieve feature parity with the AmigaOS4 Python implementation as documented in the [AmigaOS Manual: Python Modules and Packages](https://wiki.amigaos.net/wiki/AmigaOS_Manual:_Python_Modules_and_Packages).

## Current Status Analysis

### ✅ Already Implemented Functions

The current implementation already includes most core functionality:

**File System Operations:**
- `chdir()`, `chmod()`, `chown()`, `close()`, `dup()`, `dup2()`
- `fdopen()`, `fstat()`, `ftruncate()`, `lseek()`, `lstat()`
- `mkdir()`, `open()`, `read()`, `readlink()`, `rename()`
- `rmdir()`, `stat()`, `symlink()`, `unlink()`, `write()`

**Process Management:**
- `getcpu()`, `getmachine()`, `getpid()`, `getports()`
- `kill()`, `nice()`, `waitpid()`

**System Information:**
- `access()`, `getcwd()`, `getcwdu()`, `isatty()`
- `listdir()`, `pathconf()`, `fpathconf()`, `sysconf()`
- `uname()`

**Environment & Utilities:**
- `putenv()`, `strerror()`, `system()`, `tempnam()`
- `tmpfile()`, `tmpnam()`, `umask()`, `urandom()`
- `utime()`, `waitforport()`

**Time Functions:**
- `gettimeofday()`, `settimeofday()`, `sleep()`, `usleep()`

**Amiga-Specific:**
- `fullpath()`, `crc32()`

## ❌ Missing Functions to Implement

### 1. Process Management Functions

#### `abort()`
```c
static PyObject *
amiga_abort(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":abort"))
        return NULL;
    
    /* Abort the interpreter immediately */
    abort();
    /* This should never return */
    return NULL;
}
```

#### `getdtablesize()`
```c
static PyObject *
amiga_getdtablesize(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":getdtablesize"))
        return NULL;
    
    /* Return maximum number of file descriptors */
    return PyInt_FromLong((long)getdtablesize());
}
```

#### `fsync()`
```c
static PyObject *
amiga_fsync(PyObject *self, PyObject *args)
{
    int fd;
    int res;
    
    if (!PyArg_ParseTuple(args, "i:fsync", &fd))
        return NULL;
    
    Py_BEGIN_ALLOW_THREADS
    res = fsync(fd);
    Py_END_ALLOW_THREADS
    
    if (res < 0)
        return amiga_error();
    
    Py_INCREF(Py_None);
    return Py_None;
}
```

### 2. Missing POSIX Functions

#### `times()` - Process Timing
```c
static PyObject *
amiga_times(PyObject *self, PyObject *args)
{
    struct tms tms;
    clock_t result;
    
    if (!PyArg_ParseTuple(args, ":times"))
        return NULL;
    
    Py_BEGIN_ALLOW_THREADS
    result = times(&tms);
    Py_END_ALLOW_THREADS
    
    if (result == (clock_t)-1)
        return amiga_error();
    
    return Py_BuildValue("(lllll)",
        (long)result,
        (long)tms.tms_utime,
        (long)tms.tms_stime,
        (long)tms.tms_cutime,
        (long)tms.tms_cstime);
}
```

#### `mkfifo()` - Named Pipe Creation
```c
static PyObject *
amiga_mkfifo(PyObject *self, PyObject *args)
{
    char *path;
    int mode = 0666;
    int res;
    
    if (!PyArg_ParseTuple(args, "s|i:mkfifo", &path, &mode))
        return NULL;
    
    Py_BEGIN_ALLOW_THREADS
    res = mkfifo(path, mode);
    Py_END_ALLOW_THREADS
    
    if (res < 0)
        return amiga_error_with_filename(path);
    
    Py_INCREF(Py_None);
    return Py_None;
}
```

### 3. Terminal Control Functions

#### `tcgetpgrp()` - Get Terminal Process Group
```c
static PyObject *
amiga_tcgetpgrp(PyObject *self, PyObject *args)
{
    int fd;
    pid_t pgrp;
    
    if (!PyArg_ParseTuple(args, "i:tcgetpgrp", &fd))
        return NULL;
    
    Py_BEGIN_ALLOW_THREADS
    pgrp = tcgetpgrp(fd);
    Py_END_ALLOW_THREADS
    
    if (pgrp < 0)
        return amiga_error();
    
    return PyInt_FromLong((long)pgrp);
}
```

#### `tcsetpgrp()` - Set Terminal Process Group
```c
static PyObject *
amiga_tcsetpgrp(PyObject *self, PyObject *args)
{
    int fd;
    int pgrp;
    int res;
    
    if (!PyArg_ParseTuple(args, "ii:tcsetpgrp", &fd, &pgrp))
        return NULL;
    
    Py_BEGIN_ALLOW_THREADS
    res = tcsetpgrp(fd, pgrp);
    Py_END_ALLOW_THREADS
    
    if (res < 0)
        return amiga_error();
    
    Py_INCREF(Py_None);
    return Py_None;
}
```

### 4. Enhanced Error Handling Functions

#### `closerange()` - Close Range of File Descriptors
```c
static PyObject *
amiga_closerange(PyObject *self, PyObject *args)
{
    int fd_low, fd_high;
    int i;
    
    if (!PyArg_ParseTuple(args, "ii:closerange", &fd_low, &fd_high))
        return NULL;
    
    Py_BEGIN_ALLOW_THREADS
    for (i = fd_low; i < fd_high; i++) {
        close(i);  /* Ignore errors */
    }
    Py_END_ALLOW_THREADS
    
    Py_INCREF(Py_None);
    return Py_None;
}
```

## 🔧 Required Code Changes

### 1. Add Missing Function Implementations

Add the function implementations above to the source file before the `amiga_methods` array.

### 2. Update Method Table

Add the missing functions to the `amiga_methods` array:

```c
static struct PyMethodDef amiga_methods[] = {
    // ... existing methods ...
    
    /* Add missing functions */
    {"abort", amiga_abort},
    {"times", amiga_times},
    {"mkfifo", amiga_mkfifo, 1},
    {"tcgetpgrp", amiga_tcgetpgrp},
    {"tcsetpgrp", amiga_tcsetpgrp},
    {"closerange", amiga_closerange},
    
    // ... rest of existing methods ...
    {NULL, NULL}        /* Sentinel */
};
```

### 3. Add Required Headers

Add these includes at the top of the file:

```c
#include <sys/times.h>      /* For times() */
#include <sys/stat.h>       /* For mkfifo() */
#include <termios.h>        /* For tcgetpgrp/tcsetpgrp */
```

### 4. Add Missing Constants

Add these constants to the `all_ins()` function:

```c
static int all_ins(PyObject *d)
{
    // ... existing constants ...
    
    /* Add missing constants */
#ifdef S_IFIFO
    if (ins(d, "S_IFIFO", (long)S_IFIFO)) return -1;
#endif
    
#ifdef WCONTINUED
    if (ins(d, "WCONTINUED", (long)WCONTINUED)) return -1;
#endif
    
#ifdef WEXITED
    if (ins(d, "WEXITED", (long)WEXITED)) return -1;
#endif
    
#ifdef WSTOPPED
    if (ins(d, "WSTOPPED", (long)WSTOPPED)) return -1;
#endif
    
    // ... rest of existing constants ...
    return 0;
}
```

### 5. Remove Broken References

Remove or comment out these functions that are referenced but not implemented:

```c
// Remove these from amiga_methods array:
// {"execv", amiga_execv},
// {"execve", amiga_execve},
```

## 🚫 Functions Not Supported on AmigaOS

The following functions from the AmigaOS4 documentation are intentionally not supported on AmigaOS 3.2 due to platform limitations:

- `fork()` - No process forking support
- `execv()`, `execve()` - No process execution support  
- `pipe()` - Limited threading support makes pipes less useful
- `_exit()` - Use `sys.exit()` instead

## 📋 Implementation Checklist

- [ ] Add `amiga_abort()` implementation
- [ ] Add `amiga_times()` implementation  
- [ ] Add `amiga_mkfifo()` implementation
- [ ] Add `amiga_tcgetpgrp()` implementation
- [ ] Add `amiga_tcsetpgrp()` implementation
- [ ] Add `amiga_closerange()` implementation
- [ ] Add `amiga_fsync()` implementation
- [ ] Add `amiga_getdtablesize()` implementation
- [ ] Update method table with new functions
- [ ] Add required header includes
- [ ] Add missing constants to `all_ins()`
- [ ] Remove broken function references
- [ ] Test all new functions
- [ ] Update documentation

## 🔍 Testing Requirements

After implementing these changes, test the following:

1. **File Operations**: Verify `mkfifo()` creates named pipes correctly
2. **Process Timing**: Test `times()` returns valid process timing data
3. **Terminal Control**: Test `tcgetpgrp()` and `tcsetpgrp()` with console I/O
4. **Error Handling**: Verify proper error handling for unsupported operations
5. **Constants**: Ensure all new constants are available in the module

## 📚 References

- [AmigaOS Manual: Python Modules and Packages](https://wiki.amigaos.net/wiki/AmigaOS_Manual:_Python_Modules_and_Packages)
- [AmigaOS 3.2 Developer Documentation](https://wiki.amigaos.net/wiki/AmigaOS_3.2_Developer_Documentation)
- [PosixLib Documentation](https://wiki.amigaos.net/wiki/PosixLib)

## 🎯 Expected Outcome

After implementing these changes, the Amiga module will provide:

- **Complete POSIX compatibility** for supported functions
- **Proper error handling** for unsupported operations
- **Feature parity** with AmigaOS4 Python implementation
- **Enhanced system integration** for AmigaOS 3.2

The module will maintain backward compatibility while adding the missing functionality documented in the AmigaOS4 manual. 