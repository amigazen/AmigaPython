************************************************************************
*
*	amigalibsmodule_asm.asm
*
*	©1996 Irmen de Jong.
*
* Low-level assembly interface to the amiga library call,
* part of the amigalibs module from Python1.3.
* See amigalibsmodule.c for the C code of this module.
*
* This source defines 1 function: amigalibs_calllib.
* This function parses the register specification, puts the appropriate
* values into the specified registers, and calls the desired LVO.
*
* After the call, control is returned to the C code.
* D0/D1/A0/A1 are not saved.
*
*
*	HISTORY
*	~~~~~~~
*
* 16-feb-96	created.
* 18-feb-96	implemented register loading.
*
************************************************************************

	

*
*	VERY IMPORTANT NOTICE:
*	~~~~~~~~~~~~~~~~~~~~~~
*
* Due to the fact that a6 must contain the library base, and a5 the LVO-to-call,
* the following functions CAN NOT BE USED:
*
* FROM CIA.RESOURCE: (maybe if you remove a6 from the LVO by hand)
*	AddICRVector(resource,iCRBit,interrupt)(a6,d0/a1)
*	RemICRVector(resource,iCRBit,interrupt)(a6,d0/a1)
*	AbleICR(resource,mask)(a6,d0)
*	SetICR(resource,mask)(a6,d0)
*
* FROM EXEC.LIBRARY:
*	Supervisor(userFunction)(a5)
*
* FROM GRAPHICS.LIBRARY:
*	LockLayerRom(layer)(a5)
*	UnlockLayerRom(layer)(a5)
*	AttemptLockLayerRom(layer)(a5)
*



*
* ULONG
* amigalibs_calllib(register __a0 struct Library *libbase, 
*				  register __d0 LONG LVOvalue,
*				  register __d1 UWORD regspec,
*				  register __a1 ULONG *regs);
*

	SECTION amigalibs,CODE

	XDEF	_amigalibs_calllib

_amigalibs_calllib:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	a0,a5
	move.l	a0,a6
	add.l	d0,a5			; a6=libbase, a5=LVO to call
	move.l	a5,-(sp)
	move.l	a1,a5

	move.l	d1,d7
	lsr.l	#1,d7
	bcc.s	.0
	move.l	(a5),d0		; D0

.0	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.1
	move.l	(a5),d1		; D1

.1	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.2
	move.l	(a5),d2		; D2

.2	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.3
	move.l	(a5),d3		; D3

.3	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.4
	move.l	(a5),d4		; D4

.4	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.5
	move.l	(a5),d5		; D5

.5	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.6
	move.l	(a5),d6		; D6

.6	lea	4(a5),a5
	lsr.l	#1,d7
	; bcc.s	.7
	move.l	(a5),d7_value	; D7 -- special case

.7	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.8
	move.l	(a5),a0		; A0

.8	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.9
	move.l	(a5),a1		; A1

.9	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.10
	move.l	(a5),a2		; A2

.10	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.11
	move.l	(a5),a3		; A3

.11	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.12
	move.l	(a5),a4		; A4

.12
	; A5, A6, A7 ARE NOT INITIALISED!!! (for obvious reasons)

	move.l	(sp)+,a5
	move.l	d7_value(pc),d7	; get D7
	jsr	(a5)		; call library function
	movem.l	(sp)+,d2-d7/a2-a6
	rts

d7_value
	dc.l	0

	END

