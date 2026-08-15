************************************************************************
*
*	amigalibs_calllib.asm
*
*	Port of Amiga_Misc/unused/amigalibsmodule_asm.asm for VBCC.
*	VBCC passes arguments on the stack (not SAS/C register args).
*
* ULONG amigalibs_calllib(struct Library *libbase,
*                         LONG LVOvalue,
*                         UWORD regspec,
*                         ULONG *regs);
*
* Stack on entry: 0(sp)=return 4=libbase 8=LVO 12=regspec 16=regs
*
************************************************************************

	SECTION amigalibs,CODE

	XDEF	_amigalibs_calllib

_amigalibs_calllib:
	movem.l	d2-d7/a2-a6,-(sp)	; 11 longs = 44 bytes
	; After save: 44=ret 48=libbase 52=LVO 56=regspec 60=regs
	move.l	48(sp),a0		; libbase
	move.l	52(sp),d0		; LVO
	move.l	56(sp),d1		; regspec (UWORD in low)
	move.l	60(sp),a1		; regs[]

	move.l	a0,a5
	move.l	a0,a6
	add.l	d0,a5			; a6=libbase, a5=LVO entry
	move.l	a5,-(sp)		; save call address
	move.l	a1,a5			; a5 = regs pointer

	move.l	d1,d7
	lsr.l	#1,d7
	bcc.s	.0
	move.l	(a5),d0
.0	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.1
	move.l	(a5),d1
.1	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.2
	move.l	(a5),d2
.2	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.3
	move.l	(a5),d3
.3	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.4
	move.l	(a5),d4
.4	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.5
	move.l	(a5),d5
.5	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.6
	move.l	(a5),d6
.6	lea	4(a5),a5
	lsr.l	#1,d7
	; always load d7 slot into temp (original used special path)
	move.l	(a5),d7_value
.7	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.8
	move.l	(a5),a0
.8	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.9
	move.l	(a5),a1
.9	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.10
	move.l	(a5),a2
.10	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.11
	move.l	(a5),a3
.11	lea	4(a5),a5
	lsr.l	#1,d7
	bcc.s	.12
	move.l	(a5),a4
.12
	move.l	(sp)+,a5		; LVO address
	move.l	d7_value(pc),d7
	jsr	(a5)
	movem.l	(sp)+,d2-d7/a2-a6
	rts

d7_value
	dc.l	0

	END
