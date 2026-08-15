:mod:`os` --- Miscellaneous operating system interfaces (Amiga)
===============================================================

.. module:: os
   :synopsis: Portable OS interfaces (Amiga notes).


On Amiga, :mod:`os` imports the builtin :mod:`amiga` module as its OS
implementation (the role of :mod:`posix` on Unix).  Use :mod:`os` for all
portable calls.

This page records **Amiga-specific** behaviour.  For the full portable
API see https://docs.python.org/2.7/library/os.html and :doc:`amiga`.


.. data:: name
          pathsep
          sep
          pardir
          curdir
          defpath
          altsep

   On Amiga these are::

      name     = 'amiga'
      sep      = '/'
      pathsep  = ';'
      pardir   = '/'
      curdir   = ''
      defpath  = 'C:'
      altsep   = None


.. data:: environ
          globalvars
          shellvars
          shellaliases

   See :mod:`amiga` for ENV: / shell semantics.  Writes to ``os.environ``
   update ENV:.


Unsupported process APIs
-------------------------

.. function:: execl(...)
               execle(...)
               execlp(...)
               execlpe(...)
               execvp(...)
               execvpe(...)

   Raise an exception: classic Amiga has no ``fork``/``exec`` process
   model.  ``os.system`` and ``os.popen`` remain available.


.. _amiga-os-path:

:mod:`os.path` --- Amiga path operations
---------------------------------------

.. module:: os.path
   :synopsis: Common pathname manipulations (Amiga rules).


Implemented by ``amigapath`` (``posixpath`` is a stub that imports it).
Always use ``os.path``, not ``amigapath`` or ``posixpath`` directly.

Amiga path rules that differ from Unix:

* A path is **absolute** if it contains ``:`` (device or assign).
* ``//`` means **parent directory**; :func:`os.path.normpath` must **not**
  collapse doubled slashes.
* :func:`os.path.split` strips only **one** trailing ``/`` from the head
  (so a trailing ``//`` parent marker is preserved when reconstructing).
* :func:`os.path.ismount` treats a device name as a mount point.
* :func:`os.path.normcase` lowercases the path.
* There are no ``.`` / ``..`` directory entries for :func:`os.path.walk`.

.. function:: os.path.fullpath(path)

   Expand assigns to a full Amiga pathname (from :func:`amiga.fullpath`).

   Availability: Amiga.


.. seealso::

   :func:`amiga.to_unix` / :func:`amiga.from_unix`
      Optional converters for Unix-style path strings.
