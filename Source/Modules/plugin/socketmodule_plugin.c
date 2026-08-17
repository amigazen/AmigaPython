/*
 * Compile socketmodule.c as a LoadSeg plugin TU with host redirects.
 * PYAMIGA_PLUGIN_BUILD comes from the plugin vmakefile CFLAGS.
 */
#include "Python.h"
#include "pyamiga_plugin.h"
#include "pyamiga_redir.h"
#include "../socketmodule.c"
