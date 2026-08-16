:mod:`arexx` --- ARexx interface
================================

.. module:: arexx
   :platform: Amiga
   :synopsis: High-level ARexx ports, messages, and hosts.


This module provides a high-level ARexx API.  It is a Python wrapper around
the builtin accelerator :mod:`_arexx`.

**Prefer** ``import arexx``.  Do not use ``_arexx`` directly except when
implementing new wrappers.

Location: ``Lib/site-python/arexx.py`` (classic Amiga ``site-python``).


.. seealso::

   Module :mod:`amiga`
      Dos helpers and :class:`amiga.ArgParser` used by ARexx hosts.


.. exception:: error

   Exception type from :mod:`_arexx` for ARexx failures.


.. function:: errorstring(number)

   Return the text associated with an ARexx error *number*.


.. function:: dorexx(port, cmd[, scope])

   Send *cmd* to ARexx host *port*.  Returns ``(rc1, rc2, result)``.
   Optional *scope* dict is synced to/from ARexx variables (nested dicts
   map to stem.var names).

   Availability: Amiga.


Result codes
------------

.. data:: RC_OK
          RC_WARN
          RC_ERROR
          RC_FATAL

   Standard ARexx result codes (0, 5, 10, 20).


Ports
-----

.. class:: port(name)

   Low-level wrapper around an ARexx port.  Prefer
   :class:`privateport` or :class:`publicport` / :class:`Port`.

   .. attribute:: signal
   .. attribute:: name

   .. method:: close()
               wait()
               getmsg()
               send(to, cmd[, async])
               flush()
               setstringmsgs(flag)
               settokenizeline(flag)


.. class:: privateport()

   Private port for sending messages to other hosts.


.. class:: publicport([name='PYTHON'])
               Port([name='PYTHON'])

   Public host port.  *name* must be a valid ARexx port name (letters,
   digits, ``_``, ``.``).  ``Port`` is an OS4-style alias for
   ``publicport``.


.. class:: host([name='PYTHON'[, cmds=None]])

   Full ARexx host with command dispatch using
   :class:`amiga.ArgParser` templates.

   .. method:: setcommand(cmd, template, defaults, func)
               setcommands(cmds)
               setdefaults(cmd, defaults)
               defaults(cmd)
               catchExceptions(yes)
               dispatch()
               run()
               flush()
               close()


Messages
--------

Messages returned from ports are :class:`_arexx` message objects with:

.. attribute:: wantresult
               msg
               rc
               rc2
               result

.. method:: reply()
               setvar(name, value)
               getvar(name)


Utilities
---------

.. function:: SendARexxMsg(Port, Message)

   Send *Message* to *Port* via a temporary private port.

.. function:: CallARexxFunc(Func, *Args)

   Call an ARexx function through the ``REXX`` port.


.. module:: _arexx
   :synopsis: Low-level ARexx accelerator (do not import directly).

The builtin ``_arexx`` module supplies ``port``, ``error``,
``errorstring``, ``dorexx``, and message objects.  Public code should use
:mod:`arexx`.
