Amiga platform notes
====================

.. sectionauthor:: AmigaPython port (classic notes by Irmen de Jong;
                   updated for 2.7.18 by amigazen project)


This page is **not** a module reference.  It describes AmigaPython
platform behaviour.  Module APIs are in :doc:`amiga`, :doc:`os`,
:doc:`arexx`, :doc:`environment`, and :doc:`other`.


Documentation
-------------

* Language and library: https://docs.python.org/2.7/
* Amiga module docs: this directory (``Amiga_Misc/Docs``), also shipped
  under ``Python/docs/amiga`` in the distribution tree.


Module search path
------------------

``sys.path`` follows Python 2.7 rules with Amiga path syntax:

* Derived from the executable location / ``PYTHONHOME`` when set.
* ``PYTHONPATH`` is a **semicolon**-separated list of directories.
* The script directory (or ``''``) is inserted at the front.
* ``site.py`` is imported by default (disable with ``-S``).
* On Amiga, ``site`` adds ``Lib/site-python`` and ``Lib/site-packages``.

Amiga-specific Python modules (``arexx``, ``asl``, ``catalog``, ``icon``,
``_amigados``, …) live in ``Lib/site-python``.


Import case sensitivity
-----------------------

Imports match the **case** of the module filename.  Set environment
variable ``PYTHONCASEOK`` to relax this.


POSIX replacements
-----------------

* Builtin :mod:`amiga` fills the role of Unix :mod:`posix` for :mod:`os`.
* ``posixpath`` is a stub that imports Amiga path logic used by
  :mod:`os.path`.


Networking
----------

Socket features need ``bsdsocket.library`` (AmiTCP historically; modern
stacks such as Roadshow provide it).  ``pwd`` / ``grp`` / ``crypt`` may
need ``usergroup.library``.  I-Net225 is not supported.


Timezones
---------

Timezone data comes from locale preferences when available, else
``ENV:TZ`` (for example ``MET-1``).


.. data:: sys.platform

   Always ``'amiga'`` (lowercase).


Workbench startup
-----------------

* Tooltypes become argv entries (one tooltype = one argument).
* Multiselect Python + script runs the script; script tooltypes append.
* Magic tooltypes: ``PYTHONSCRIPT=...``, ``PYSCRIPTARG=...``.
* Default tool for scripts: ``Python:Python``.
* Icon-only ``-c`` scripts are supported (tooltype ``-c`` plus next
  tooltype = code).


Library files
-------------

Do not change the portable behaviour of standard library modules.  Put
Amiga-only modules in ``Lib/site-python``.


Building / sources
------------------

See ``Source/README.AMIGA`` and project ``BUILD.md``.  Contact:
amigazen project (https://github.com/amigazen/amigapython/).
