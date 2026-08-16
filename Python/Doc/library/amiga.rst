:mod:`amiga` --- Amiga operating system interfaces
================================================

.. module:: amiga
   :platform: Amiga
   :synopsis: Amiga operating system interfaces (used via os).


This module provides access to AmigaDOS and related system facilities.  It
is the Amiga counterpart of the Unix :mod:`posix` module.

**Do not import this module for portable OS calls.**  Import :mod:`os`
instead, which re-exports the portable surface.  Use ``import amiga`` only
for Amiga-specific APIs (Dos.library helpers, ASL, catalogs, icons, CRC-32,
path conversion).

Errors from OS calls are reported as :exc:`OSError` (aliased as
:exc:`amiga.error`).  Dos.library helpers raise :exc:`amiga.doserror`.


.. seealso::

   Module :mod:`os`
      Portable operating system interface (preferred for most code).

   Module :mod:`os.path`
      Amiga path operations (``:`` absolute, ``//`` parent directory).

   Module :mod:`arexx`
      High-level ARexx interface.

   Module :mod:`environment`
      Fine-grained ENV: and shell variable control.


Module layout
-------------

On AmigaPython 2.7.18 the builtin ``amiga`` module is assembled from:

* POSIX-style calls in ``Modules/amiga/amigamodule.c``
* Dos.library entry points (former Doslib) via ``amiga_init_dos()``
* Path helpers ``to_unix`` / ``from_unix``
* ASL / Intuition helpers (``FileRequest``, ``MessageBox``)
* Locale catalogs (``OpenCatalog``)
* icon.library (``DiskObject``)
* Python helpers from ``Lib/site-python/_amigados.py`` (``ArgParser``,
  ``FIBF_*``, ``touch``, …), installed by ``site`` after import bootstrap

Thin OS4-compatible shims in ``Lib/site-python/``::

   import asl       # FileRequest, MessageBox
   import catalog   # OpenCatalog
   import icon      # DiskObject
   import dos       # legacy name; re-exports from amiga


.. _amiga-exceptions:

Exceptions
----------

.. exception:: error

   Alias for :exc:`OSError`.

.. exception:: doserror

   Raised by Dos.library helpers (ReadArgs, Examine, …).


.. _amiga-data:

Data
----

.. data:: environ

   Mapping of **global (ENV:) and local (shell)** variables.  Local shell
   values override globals when both exist.  Assignments update ENV:
   (they do not update an existing shell local of the same name).

.. data:: globalvars

   Read-only mapping of ENV: variables only.

.. data:: shellvars

   Read-only mapping of local shell variables only.

.. data:: shellaliases

   Read-only mapping of shell aliases (when available).

.. data:: name

   Always ``'amiga'`` when used through :mod:`os` (see ``os.name``).

Open flags (``O_*``), access modes (``F_OK``, ``R_OK``, …), wait flags
(``WNOHANG``, …), and selected ``_PC_*`` / ``_SC_*`` constants are
registered when the C headers provide them.


.. _amiga-posix:

POSIX-style functions
-------------------

Unless noted, behaviour matches :mod:`os` / :mod:`posix`.  Prefer calling
these through :mod:`os`.

Filesystem
^^^^^^^^^^

.. function:: chdir(path)
               chmod(path, mode)
               chown(path, uid, gid)
               getcwd()
               listdir(path)
               mkdir(path[, mode])
               rename(src, dst)
               rmdir(path)
               remove(path)
               unlink(path)
               link(src, dst)
               symlink(src, dst)
               readlink(path)
               stat(path)
               lstat(path)
               utime(path, (atime, mtime))
               access(path, mode)
               fullpath(path)

   Expand assigns to a full Amiga path.  Prefer :func:`os.path.fullpath`.

   Availability: Amiga.

File descriptors
^^^^^^^^^^^^^^^^

.. function:: open(file, flags[, mode])
               close(fd)
               read(fd, n)
               write(fd, str)
               lseek(fd, pos, how)
               fstat(fd)
               fdopen(fd[, mode[, bufsize]])
               ftruncate(fd, length)
               fsync(fd)
               isatty(fd)
               closerange(fd_low, fd_high)
               dup(fd)
               dup2(fd, fd2)

   ``dup`` / ``dup2`` currently apply to socket descriptors and need
   ``bsdsocket.library``.

Process / identity
^^^^^^^^^^^^^^^^^^

.. function:: getpid()
               getppid()
               getuid()
               geteuid()
               getgid()
               getegid()
               getpgrp()
               setuid(uid)
               setgid(gid)
               setpgrp()
               setpgid(pid, pgrp)
               setsid()
               umask(mask)
               nice(inc)
               kill(pid, sig)
               waitpid(pid, options)
               abort()
               _exit(n)
               popen(command[, mode[, bufsize]])
               system(command)
               uname()
               getcpu()
               getmachine()

   ``getpid()`` returns ``FindTask(NULL)`` as an integer address.
   ``getcpu()`` / ``getmachine()`` are Amiga identity helpers (OS4-style).
   User/group calls need ``usergroup.library`` when the C library requires it.

Configuration / utility
^^^^^^^^^^^^^^^^^^^^^^^

.. function:: pathconf(path, name)
               fpathconf(fd, name)
               sysconf(name)
               getdtablesize()
               tempnam([dir[, prefix]])
               urandom(n)
               strerror(code)
               putenv(str)
               sleep(seconds)
               usleep(useconds)
               gettimeofday()
               settimeofday(sec, usec)
               pipe()
               set_verbose(flag)

   ``pipe()`` may be unavailable or limited without thread support.
   ``urandom`` uses a platform RNG (not a hardware TRNG unless provided).


Not supported
^^^^^^^^^^^^^

The following are not useful or not implemented on classic Amiga and remain
unavailable or raise errors: ``fork``, ``execv``, ``execve``, ``times``,
``mkfifo``, ``tcgetpgrp``, ``tcsetpgrp``.


.. _amiga-amigaonly:

Amiga-specific functions
-----------------------

.. function:: crc32(string[, start])

   Return the CRC-32 checksum of *string*.  Optional *start* continues a
   previous checksum.

   Availability: Amiga.

.. function:: to_unix(path)
               from_unix(path)

   Convert between Amiga ``device:path`` form and a Unix-like
   ``/device/path`` form.  For normal path work use :mod:`os.path`.

   Availability: Amiga.


.. _amiga-asl:

ASL and Intuition
-----------------

Requires ``asl.library`` / ``intuition.library`` (opened on demand).

.. function:: FileRequest([title[, drawer[, filename[, pattern]]]])

   Pop up an ASL file requester.  Returns ``(drawer, filename)`` or
   ``None`` if cancelled.  Keyword arguments: ``title``, ``drawer``,
   ``filename``, ``pattern``.

   Also available as :func:`asl.FileRequest`.

   Availability: Amiga.

.. function:: MessageBox(title, body, gadgets)

   EasyRequest-style message box.  Returns the 0-based gadget index
   chosen by the user.

   Also available as :func:`asl.MessageBox`.

   Availability: Amiga.


.. _amiga-catalog:

Locale catalogs
---------------

Requires ``locale.library``.

.. function:: OpenCatalog(name[, languagename[, builtinlanguage]])

   Open a catalog file.  Returns a :class:`Catalog` object.

   Also available as :func:`catalog.OpenCatalog`.

   Availability: Amiga.

.. class:: Catalog

   .. method:: GetString(id[, default])

      Return catalog string *id*, or *default* if missing.


.. _amiga-icon:

Icons
-----

Requires ``icon.library``.

.. function:: DiskObject(name)

   Load a disk icon (do **not** include the ``.info`` suffix).  Returns a
   :class:`DiskObject` instance.

   Also available as :func:`icon.DiskObject`.

   Availability: Amiga.

.. class:: DiskObject

   .. attribute:: tooltypes

      List of tooltype strings (read/write).

   .. attribute:: deftool

      Default tool path (read/write).

   .. attribute:: stacksize

      Stack size (read/write).

   .. method:: PutIcon(name)

      Write the DiskObject back to disk (without ``.info`` suffix).


.. _amiga-dos:

Dos.library helpers
-----------------

These entry points (historically the Doslib / Dos modules) are registered
on ``amiga``.  Prefer ``import amiga`` over a separate Doslib module.

Signals and args
^^^^^^^^^^^^^^^^

.. function:: ReadArgs(...)

   Low-level ReadArgs.  Prefer :class:`amiga.ArgParser`.

.. function:: WaitSignal(what)
               CheckSignal(what)

   Wait for or poll AmigaDOS signals.  *what* may be an integer mask, an
   object with a ``signal`` attribute, or a list of those.  Returns
   ``(sigs, objlist)``.  Detected signals in the wait set are cleared.


Dates and faults
^^^^^^^^^^^^^^^^

.. function:: DateStamp()
               CompareDates(d1, d2)
               DateToStr(date[, format[, flags]])
               StrToDate(dstr[, timestring[, format[, flags]]])
               DS2time(ds)
               time2DS(seconds)
               Fault(err[, header])
               IoErr()
               SetIoErr(err)


Program and volume
^^^^^^^^^^^^^^^^^^

.. function:: GetProgramDir()
               GetProgramName()
               Inhibit(drive[, switch])
               IsFileSystem(path)
               Relabel(oldvolname, newvolname)
               Info(drivename)
               Examine(file)
               SetProtection(file, protbits)
               SetComment(file, comment)
               SetFileDate(file, datestamp)
               SetOwner(file, uid, gid)


Console (raw mode)
^^^^^^^^^^^^^^^^^^

.. function:: SetMode(mode)
               WaitForChar(timeout)
               GetChar()
               PutChar(char)
               PutString(string)

   *mode* is ``MODE_CON`` or ``MODE_RAW``.


Python Dos helpers (from ``_amigados``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Installed onto ``amiga`` by ``site``:

.. function:: touch(file[, time])
               AddBuffers(drive, buffers)
               AssignAdd(name, target)
               AssignRemove(name)

.. class:: ArgParser(template)

   Amiga ReadArgs-style argument parser.

   .. attribute:: defaults
   .. attribute:: template
   .. attribute:: types

   .. method:: new(template)
               reset()
               parse(args)

      Return a dict of parsed arguments.


Constants include ``SIGBREAKF_CTRL_C`` … ``F``, ``FIBF_*``,
``FORMAT_DOS`` / ``FORMAT_INT`` / ``FORMAT_USA`` / ``FORMAT_CDN``,
``MODE_CON`` / ``MODE_RAW``, and ``fib_*`` / ``id_*`` tuple indexes for
:func:`Examine` / :func:`Info` results.
