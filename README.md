# Amiga Python

This is Amiga Python, a port of Python 2 to Amiga.

## [amigazen project](http://www.amigazen.com)

*A web, suddenly*

*Forty years meditation*

*Minds awaken, free*

**amigazen project** uses modern software development tools and methods to update and rerelease classic Amiga open source software. Our releases include a new AWeb, this new Amiga Python 2, and the ToolKit project - a universal SDK for Amiga.

Key to our approach is ensuring every project can be built with the same common set of development tools and configurations, so we created the ToolKit project to provide a standard configuration for Amiga development. All *amigazen project* releases will be guaranteed to build against the ToolKit standard so that anyone can download and begin contributing straightaway without having to tailor the toolchain for their own setup.

The original authors of the *Python* software and its Amiga port are not affiliated with the amigazen project. This software is redistributed on terms described in the documentation, particularly the file LICENSE.md

Our philosophy is based on openness:

*Open* to anyone and everyone	- *Open* source and free for all	- *Open* your mind and create!

PRs for all of our projects are gratefully received at [GitHub](https://github.com/amigazen/). While our focus now is on classic 68k software, we do intend that all amigazen project releases can be ported to other Amiga-like systems including AROS and MorphOS where feasible.

## About Amiga Python

Python was originally ported to and adapted for the Amiga in 1999-2000 by Irmen de Jong. All of his original releases to Aminet up to and inclide Python 2.0 are included here with original source code in this git repo's commit history and archive folder.

This project aims to continue his legacy by updating Amiga Python to the final 2.7.18 revision of Python 2, ensuring it can be built out of the box against the ToolKit standard by anyone with an Amiga computer.

Amiga Python has always aimed to be as complete a port of Python as it makes sense to be for the Amiga platform, and includes Amiga specific builtin modules for native functionality including the _amiga_ and _arexx_ modules.

In the process of updating Amiga Python to 2.7.18, amigazen project intends to make the following changes to reflect the reality of building and running Amiga software in 2025:

- Remove support for INet225 and update AmiTCP support to reflect the RoadShow version of bsdsocket.library
- While keeping SAS/C compiler support is desirable, pragmatically the build will probably need VBCC or GCC, and in doing so replace the POSIX dependencies with a more complete C library implementation
- Explore further enhancements to Amiga Python such as exporting the main interpreter as a shared library, and leveraging shared libraries instead of static link libraries for zlib and bzip2 and other builtin functionality, as well as a new SlimPython runtime for every day operating system automation

Note that this Amiga Python project is not currently and has never been associated with the version of Python included in OS4, however future collaboration on updating that version, as well as versions for other Amiga-like platforms, is very welcome, in the amigazen spirit of openness.

## About ToolKit

**ToolKit** exists to solve the problem that most Amiga software was written in the 1980s and 90s, by individuals working alone, each with their own preferred setup for where their dev tools are run from, where their include files, static libs and other toolchain artifacts could be found, which versions they used and which custom modifications they made. Open source collaboration did not exist as we know it in 2025. 

**ToolKit** from amigazen project is a work in progress to make a standardised installation of not just the Native Developer Kit, but the compilers, build tools and third party components needed to be able to consistently build projects in collaboration with others, without each contributor having to change build files to work with their particular toolchain configuration. 

All *amigazen project* releases will release in a ready to build configuration according to the ToolKit standard.

Each component of **ToolKit** is open source and will have it's own github repo, while ToolKit itself will eventually be released as an easy to install package containing the redistributable components, as well as scripts to easily install the parts that are not freely redistributable from archive.

## Contact 

- At GitHub https://github.com/amigazen/amigapython/ 
- on the web at http://www.amigazen.com/amigapython/ (Amiga browser compatible)
- or email amigapython@amigazen.com

## [Aminet.readme](https://www.aminet.net/package/dev/lang/Python20)

***N.B. this readme contents dates to 2000! contact details may no longer be relevant!***

AMIGAPYTHON 2.0

                           RELEASE 2.0 (build 1)

	
           Conversion and Amiga specific code by Irmen de Jong.
              (Original code by Guido van Rossum and others)



* WHAT'S THIS?

This is the Python interpreter (version 2.0) for AmigaDOS.  If you don't
know what Python is and what it can do for you, visit the Python homepage
at <http://www.python.org/>.

AmigaPython is a fully Unicode compatible application and includes a
fast XML parser (Expat). You will need some RAM but when you want to
process Unicode text and/or XML documents, AmigaPython is your tool!


* WHAT'S IN THIS ARCHIVE?

    - Python 2.0 binary for 68030+FPU
    - Installer® installation script (in Dutch and English)
    - Some demo programs & icons
    - Python library modules
    - Some documentation, mostly on Amiga features


* MINIMAL SYSTEM REQUIREMENTS	

    - 4 megabytes of memory. Probably more.
    - Harddisk.
    - Kickstart 2.04
    - 68030 CPU, FPU required
    - For networking functions: AmiTCP (Version 4)
      (version 3 might work too, but I haven't checked this)
    - For user authorisation stuff: usergroup.library from AmiTCP or
      MultiUserFileSystem.


* CHANGES SINCE THE PREVIOUS VERSION (version 1.x):

  Well, this is the new version: 2.0! It includes a fast XML parser (Expat).
  Be aware that there are a few language incompatibilities. Just check
  out the regular Python 2.0 news on www.python.org to find out what they are.


Check the README file in the archive for more information.  You can also
find more information on Python at my AmigaPython Web page;
<http://www.bigfoot.com/~irmen/python.html>


IMPORTANT:
Please let me know if you find any errors, encounter problems, or have any
suggestions!  But, as I work on this in my spare time, don't expect
miracles...

SUGGESTION:
Unpack the archive to where you want the program to be.  The installation
will then be performed much quicker.
The 2.0 installer also lets you install Python 2.0 next to a previous 1.x
installation, so you can use them together.

SOURCE?
Also on Aminet, where you got this:  dev/lang/Python20_Src.lha.



					Irmen de Jong

## Acknowledgements

*Amiga* is a trademark of **Amiga Inc**. 

Original Amiga Python by Irmen de Jong

Python is a product of the Python Software Foundation, see LICENSE.md