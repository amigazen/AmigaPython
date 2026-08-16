#
# AMIGA LIBRARY INTERFACE FOR intuition.library
# Secondary FFI example (amigalibs + ConvertFD).
# Prefer curated amiga.EasyRequest / CurrentTime / DisplayBeep and amigagui.
#
# LVO table originally from ConvertFD / FD:intuition_lib.fd (subset used by demos).
# Regenerate fuller tables with: python Tools/ConvertFD.py intuition
#

LVO = {
    "ActivateWindow": (-450, 256),
    "ChangeWindowBox": (-486, 271),
    "CloseScreen": (-66, 256),
    "CloseWindow": (-72, 256),
    "CurrentTime": (-84, 768),
    "DisplayBeep": (-96, 256),
    "DrawBorder": (-108, 771),
    "EasyRequestArgs": (-588, 3840),
    "OpenScreen": (-198, 256),
    "OpenScreenTagList": (-612, 768),
    "OpenWindow": (-204, 256),
    "OpenWindowTagList": (-606, 768),
    "PrintIText": (-216, 771),
    "SetWindowTitles": (-276, 1792),
}

import amigalibs

libname = "intuition.library"
lib = amigalibs.openlib(libname, 37)

IntuitionlibError = "IntuitionlibError"


def call(func, args):
    try:
        return lib.call(LVO[func], args)
    except KeyError:
        raise NameError, func + " not found in " + libname


# Common tags / flags (hand subset; extend via h2py on NDK headers).
TAG_DONE = TAG_END = 0
TAG_IGNORE = 1
TAG_USER = 0x80000000

WA_Dummy = TAG_USER + 99
WA_Left = WA_Dummy + 0x01
WA_Top = WA_Dummy + 0x02
WA_Width = WA_Dummy + 0x03
WA_Height = WA_Dummy + 0x04
WA_IDCMP = WA_Dummy + 0x07
WA_Flags = WA_Dummy + 0x08
WA_Title = WA_Dummy + 0x0B
WA_MinWidth = WA_Dummy + 0x0F
WA_MinHeight = WA_Dummy + 0x10
WA_MaxWidth = WA_Dummy + 0x11
WA_MaxHeight = WA_Dummy + 0x12
WA_InnerWidth = WA_Dummy + 0x13
WA_InnerHeight = WA_Dummy + 0x14

WFLG_SIZEGADGET = 0x00000001
WFLG_DRAGBAR = 0x00000002
WFLG_DEPTHGADGET = 0x00000004
WFLG_CLOSEGADGET = 0x00000008
WFLG_ACTIVATE = 0x00001000
WFLG_NEWLOOKMENUS = 0x00200000
WFLG_DEFAULT = (WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET |
                WFLG_CLOSEGADGET | WFLG_ACTIVATE | WFLG_NEWLOOKMENUS)

IDCMP_CLOSEWINDOW = 0x00000200
IDCMP_VANILLAKEY = 0x00200000
IDCMP_MOUSEBUTTONS = 0x00000008


def DisplayBeep(screen=None):
    # Prefer amiga.DisplayBeep for curated API.
    addr = 0 if screen is None else int(screen)
    return call("DisplayBeep", {8: addr})
