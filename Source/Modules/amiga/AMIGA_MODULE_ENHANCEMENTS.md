# Amiga Module Enhancements for Python 2.7.18

## Overview

This document describes the enhancements made to `amigamodule.c` to provide better POSIX compatibility for Python 2.7.18 on AmigaOS using VBCC and PosixLib.

## New Functions Added

The following POSIX functions have been added to amigamodule, using PosixLib implementations where available:

### File System Functions
- `access(path, mode)` - Check file accessibility (F_OK, R_OK, W_OK, X_OK)
- `isatty(fd)` - Check if file descriptor is a terminal
- `closerange(fd_low, fd_high)` - Close range of file descriptors
- `fsync(fd)` - Synchronize file data to disk
- `getdtablesize()` - Get maximum number of file descriptors

### Process Management
- `nice(incr)` - Set process priority
- `kill(pid, sig)` - Send signal to process
- `waitpid(pid, options)` - Wait for child process
- `sleep(seconds)` - Sleep for specified seconds
- `usleep(useconds)` - Sleep for specified microseconds

### System Configuration
- `pathconf(path, name)` - Get path configuration limits
- `fpathconf(fd, name)` - Get file configuration limits
- `sysconf(name)` - Get system configuration limits

### Time Functions
- `gettimeofday()` - Get current time with microsecond precision
- `settimeofday(sec, usec)` - Set system time

### Utility Functions
- `tempnam(dir, prefix)` - Generate temporary filename
- `urandom(size)` - Get random bytes (Amiga implementation using rand())

## Constants Added

The following POSIX constants are now available in the amiga module:

### Access Mode Constants
- `F_OK` - File exists
- `R_OK` - File is readable
- `W_OK` - File is writable
- `X_OK` - File is executable

### Wait Options
- `WNOHANG` - Don't block if no child has exited
- `WUNTRACED` - Also return if a child has stopped

### Path Configuration Constants
- `_PC_PATH_MAX` - Maximum path length
- `_PC_NAME_MAX` - Maximum filename length
- `_PC_LINK_MAX` - Maximum number of links

### System Configuration Constants
- `_SC_ARG_MAX` - Maximum argument list length
- `_SC_CHILD_MAX` - Maximum number of child processes
- `_SC_OPEN_MAX` - Maximum number of open files
- `_SC_PAGESIZE` - System page size

## Implementation Details

### PosixLib Integration
Most new functions use PosixLib implementations, which provide:
- Proper AmigaOS integration
- BSD socket compatibility
- POSIX-compliant behavior
- Error handling consistent with AmigaOS

### Amiga-Specific Implementations
Some functions have Amiga-specific implementations:
- `urandom()` - Uses Amiga's rand() function (could be enhanced with random.library)
- `closerange()` - Simple loop implementation that ignores errors
- `gettimeofday()` - Uses PosixLib's time functions

### Error Handling
All functions follow the established amigamodule error handling pattern:
- Use `amiga_error()` for general errors
- Use `amiga_error_with_filename()` for file-related errors
- Proper thread safety with `Py_BEGIN_ALLOW_THREADS`/`Py_END_ALLOW_THREADS`

## Build Requirements

To build the enhanced amigamodule, you need:
- VBCC compiler with AmigaOS target
- PosixLib library
- Python 2.7.18 source code
- AmigaOS 3.2 SDK

## Usage Example

```python
import amiga

# Check file accessibility
if amiga.access("myfile.txt", amiga.R_OK):
    print("File is readable")

# Get system information
max_open = amiga.sysconf(amiga._SC_OPEN_MAX)
print(f"Maximum open files: {max_open}")

# Sleep for 1 second
amiga.sleep(1)

# Get current time
sec, usec = amiga.gettimeofday()
print(f"Current time: {sec}.{usec}")
```

## Compatibility Notes

- All new functions are conditionally compiled with `#ifdef HAVE_*` guards
- Functions gracefully degrade if PosixLib is not available
- Maintains backward compatibility with existing amigamodule code
- Follows AmigaOS coding conventions and standards

## Future Enhancements

Potential future improvements:
- Better random number generation using Amiga's random.library
- Enhanced process management with AmigaOS-specific features
- Network socket support using bsdsocket.library
- Thread support for pipe() and other multi-process functions
- Additional POSIX functions as needed by Python applications 