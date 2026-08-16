:mod:`environment` --- Amiga environment variables
================================================

.. module:: environment
   :platform: Amiga
   :synopsis: ENV: and local shell variable access.


This Amiga-specific builtin module manages **global** (``ENV:``) and
**local** (shell) variables.  It is not required to be POSIX-compatible;
portable code should prefer :data:`os.environ` / :func:`os.putenv`.

For OS4-style names, see :mod:`amigavars`.


Global (ENV:) variables
-----------------------

.. function:: putenv(str)

   Set a global variable.  *str* has the form ``"name=value"``.

.. function:: getenv(name)

   Return the value of ENV: variable *name*, or ``None``.

.. function:: setenv(name, value, overwrite)

   Set ENV: variable *name* to *value*.  *overwrite* is true to replace
   an existing value.

.. function:: unsetenv(name)

   Remove ENV: variable *name*.


Local (shell) variables
--------------------

.. function:: getvar(name)
               setvar(name, value, overwrite)
               unsetvar(name)

   Operate on local shell variables (not ENV:).


OS4-compatible names
---------------------

.. function:: GetEnv(name)
               SetEnv(name, value[, save])
               UnSetEnv(name[, delete])

   Same roles as getenv/setenv/unsetenv with optional ENVARC: persistence
   when *save* / *delete* are true.


.. module:: amigavars
   :synopsis: OS4-style aliases for environment GetEnv/SetEnv/UnSetEnv.

.. function:: amigavars.GetEnv(name)
               amigavars.SetEnv(name, value[, save])
               amigavars.UnSetEnv(name[, delete])

   Re-exports from :mod:`environment` (``Lib/site-python/amigavars.py``).


.. seealso::

   :data:`os.environ`, :data:`amiga.environ`, :data:`amiga.globalvars`,
   :data:`amiga.shellvars`
