Amiga notes for other standard modules
========================================

.. seealso::

   Official docs: https://docs.python.org/2.7/library/


:mod:`select`
-------------

.. module:: select
   :synopsis: Waiting for I/O completion (Amiga extension).


The Amiga :func:`select.select` accepts an optional **5th** argument: an
AmigaDOS signal mask (for example ``4096`` for Ctrl-C /
``amiga.SIGBREAKF_CTRL_C``).

When that mask is non-zero, :func:`select.select` returns a **4-tuple**
``(rlist, wlist, xlist, sigmask)`` instead of the usual 3-tuple.  The
fourth element is the mask of signals that occurred (or 0).

Pass ``None`` as the timeout (4th argument) to wait without a time
limit while still watching AmigaDOS signals.

Availability: Amiga extension to the standard API.


:mod:`time`
-----------

.. function:: time.sleep(seconds)

   If ``bsdsocket.library`` is available, sleep uses ``select`` and can be
   interrupted by Ctrl-C.  Otherwise it uses dos.library ``Delay`` and
   cannot be aborted that way.

:func:`amiga.sleep` / :func:`amiga.usleep` are also available for
posix-style callers.


:mod:`tempfile`
-------------

Temporary directory search order:

1. ``ENV:TMPDIR`` if set and valid
2. ``T:`` (usually ``RAM:T``)
3. ``:T`` (``T`` in the root of the current device)
4. ``SYS:T``


Networking helpers
------------------

``urllib``, ``urllib2``, ``httplib``, and related modules need
``bsdsocket.library``.  Behaviour follows Python 2.7 where the stack
allows; Amiga path and ENV: rules still apply for local ``file:`` URLs.
