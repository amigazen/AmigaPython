#
# AMIGA LIBRARY INTERFACE FOR intuition.library
#
# Generated from FD file FD:intuition_lib.fd by ConvertFD.py
#

# LVO definitions:

LVO = {
'ActivateWindow':(-450, 256),
'ChangeWindowBox':(-486, 271),
'CloseScreen':(-66, 256),
'CloseWindow':(-72, 256),
'CurrentTime':(-84, 768),
'DisplayBeep':(-96, 256),
'DrawBorder':(-108, 771),
'EasyRequestArgs':(-588, 3840),
'OpenScreen':(-198, 256),
'OpenScreenTagList':(-612, 768),
'OpenWindow':(-204, 256),
'OpenWindowTagList':(-606, 768),
'PrintIText':(-216, 771),
'SetWindowTitles':(-276, 1792),
}


import amigalibs
libname="intuition.library"

lib = amigalibs.openlib(libname,37)

# General libcall interface:

def call(func,args):
        try:
                return lib.call(LVO[func],args)
        except KeyError:
                raise NameError,func+" not found in "+libname


# exception:

IntuitionError = "IntuitionError"


################ USER CODE FOLLOWS

import struct

strptr = amigalibs.strptr
intptr = amigalibs.intptr
obj2reg = amigalibs.obj2reg


#
# implemented functions:
#
# EasyRequest
# CurrentTime
#
# implemented classes:
#
# Window
#


def EasyRequest(body,buttons="Ok|Cancel",title="Requester"):
    req = struct.pack("lllll",20,0,strptr(title),strptr(body),strptr(buttons))
    return call('EasyRequestArgs',{8:0,9:strptr(req),10:0,11:0})

def CurrentTime():
	secs=0
	micros=1
	call('CurrentTime',{8:intptr(secs),9:intptr(micros)})
	return (secs,micros)

TAG_DONE = TAG_END = 0
TAG_IGNORE = 1
TAG_USER = 0x80000000

WA_Dummy		= TAG_USER+99
WA_Left			= WA_Dummy + 0x01
WA_Top			= WA_Dummy + 0x02
WA_Width		= WA_Dummy + 0x03
WA_Height		= WA_Dummy + 0x04
WA_IDCMP		= WA_Dummy + 0x07
WA_Flags		= WA_Dummy + 0x08
WA_Gadgets		= WA_Dummy + 0x09
WA_Title		= WA_Dummy + 0x0B
WA_ScreenTitle	= WA_Dummy + 0x0C
WA_CustomScreen	= WA_Dummy + 0x0D
WA_MinWidth		= WA_Dummy + 0x0F
WA_MinHeight	= WA_Dummy + 0x10
WA_MaxWidth		= WA_Dummy + 0x11
WA_MaxHeight	= WA_Dummy + 0x12
WA_InnerWidth	= WA_Dummy + 0x13
WA_InnerHeight	= WA_Dummy + 0x14
	
WFLG_SIZEGADGET		= 0x00000001
WFLG_DRAGBAR		= 0x00000002
WFLG_DEPTHGADGET	= 0x00000004
WFLG_CLOSEGADGET	= 0x00000008
WFLG_SIZEBRIGHT		= 0x00000010
WFLG_SIZEBBOTTOM	= 0x00000020
WFLG_SMART_REFRESH	= 0x00000000
WFLG_SIMPLE_REFRESH	= 0x00000040
WFLG_REPORTMOUSE	= 0x00000200
WFLG_GIMMEZEROZERO	= 0x00000400
WFLG_ACTIVATE		= 0x00001000
WFLG_RMBTRAP		= 0x00010000
WFLG_NEWLOOKMENUS	= 0x00200000

WFLG_DEFAULT = WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE | WFLG_NEWLOOKMENUS

IDCMP_SIZEVERIFY	= 0x00000001
IDCMP_NEWSIZE		= 0x00000002
IDCMP_REFRESHWINDOW	= 0x00000004
IDCMP_MOUSEBUTTONS	= 0x00000008
IDCMP_MOUSEMOVE		= 0x00000010
IDCMP_GADGETDOWN	= 0x00000020
IDCMP_GADGETUP		= 0x00000040
IDCMP_MENUPICK		= 0x00000100
IDCMP_CLOSEWINDOW	= 0x00000200
IDCMP_RAWKEY		= 0x00000400
IDCMP_MENUVERIFY	= 0x00002000
IDCMP_DISKINSERTED	= 0x00008000
IDCMP_DISKREMOVED	= 0x00010000
IDCMP_ACTIVEWINDOW	= 0x00040000
IDCMP_INACTIVEWINDOW= 0x00080000
IDCMP_DELTAMOVE		= 0x00100000
IDCMP_VANILLAKEY	= 0x00200000
IDCMP_INTUITICKS	= 0x00400000
IDCMP_CHANGEWINDOW	= 0x02000000

IDCMP_DEFAULT = 0

class Window:
	# member data:
	# win

	def __init__(self,x,y,w,h,title,flags=WFLG_DEFAULT,idcmp=IDCMP_DEFAULT):
		wintags = struct.pack("2l2l2l2l2l2l2l2l2l2l2ll",
			WA_Left,x,WA_Top,y,WA_Width,w,WA_Height,h,WA_Title,strptr(title),
			WA_Flags,flags,WA_IDCMP,idcmp,WA_MinWidth,20,WA_MinHeight,20,
			WA_MaxWidth,-1,WA_MaxHeight,-1,
			TAG_DONE)
		self.win=call('OpenWindowTagList',{8:0,9:strptr(wintags)})
		if not self.win:
			raise IntuitionError,"can't open window "+title
	
	def close(self):
		if self.win:
			call('CloseWindow',{8:self.win})
			self.win=0
			
	def __del__(self):
		self.close()

