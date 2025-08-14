
	SECTION rexxvars,CODE

	XDEF    CheckRexxMsg,@CheckRexxMsg,_CheckRexxMsg
	XDEF    GetRexxVar,@GetRexxVar,_GetRexxVar
	XDEF    SetRexxVar,@SetRexxVar,_SetRexxVar


_CheckRexxMsg:
	move.l  4(sp),a0
@CheckRexxMsg:
CheckRexxMsg:
	movem.l d2/a2/a6,-(sp)
	move.l  a0,a2
	move.l  4.w,a6
	lea     rexxlibTxt(pc),a1
	moveq   #0,d0
	jsr     -$228(a6)
	move.l  d0,d2
	beq.b   skip
	move.l  d0,a1
	jsr     -$19E(a6)
	moveq   #0,d0
	cmp.l   $18(a2),d2
	bne.b   skip
	move.l  $14(a2),d1
	beq.b   skip
	move.l  a2,a0
	move.l  d2,a6
	jsr     -$A8(a6)
skip    tst.l   d0
	movem.l (sp)+,d2/a2/a6
	rts

_GetRexxVar:
	movem.l 4(sp),a0/a1
	move.l  12(sp),4(sp)
@GetRexxVar:
	bsr.b   GetRexxVar
	bne.b   lab58
	move.l  4(sp),a0
	move.l  a1,(a0)
lab58   rts

GetRexxVar:
	movem.l d2/d3/a2-a6,-(sp)
	move.l  a0,a2
	move.l  a1,a3
	bsr.b   CheckRexxMsg
	beq.b   lab96
	move.l  $18(a2),a6
	move.l  $14(a2),a0
	jsr     -$6C(a6)
	move.l  a0,a4
	move.l  a3,a0
	bsr     lab13A
	bne.b   lab98
	move.l  a1,a2
	move.l  d1,d2
	move.l  a4,a0
	move.l  d2,d0
	moveq   #0,d1
	jsr     -$48(a6)
	moveq   #0,d0
	addq.l  #8,a1
	tst.l   d1
	beq.b   lab98
	sub.l   a1,a1
	bra.b   lab98

lab96   moveq   #10,d0
lab98   tst.l   d0
	movem.l (sp)+,d2/d3/a2-a6
	rts

_SetRexxVar:
	movem.l 4(sp),a0/a1
	move.l  $10(sp),d0
	move.l  12(sp),4(sp)
@SetRexxVar:
	move.l  d0,d1
	move.l  4(sp),d0
SetRexxVar:
	movem.l d2-d7/a2-a6,-(sp)
	lea     -12(sp),sp
	move.l  a0,a2
	move.l  a1,a3
	move.l  d0,a5
	move.l  d1,d3
	lea     lab120(pc),a0
	move.l  sp,a1
	bsr     lab1F2
	move.l  a2,a0
	bsr     CheckRexxMsg
	beq.b   lab124
	move.l  $18(a2),a6
	moveq   #9,d0
	cmp.l   #$FFFF,d3
	bgt.b   lab126
	move.l  $14(a2),a0
	jsr     -$6C(a6)
	move.l  a0,a4
	move.l  a3,a0
	bsr.b   lab13A
	bne.b   lab126
	move.l  a1,a2
	move.l  d1,d2
	move.l  a4,a0
	move.l  d2,d0
	jsr     -$42(a6)
	move.l  d0,d4
	move.l  a4,a0
	move.l  a5,a1
	move.l  d3,d0
	bsr     lab1A4
	beq.b   lab120
	move.l  a4,a0
	move.l  d0,a1
	move.l  d4,d0
	jsr     -$54(a6)
	moveq   #0,d0
	bra.b   lab126

lab120  moveq   #3,d0
	bra.b   lab126

lab124  moveq   #10,d0
lab126  move.l  sp,a0
	move.l  d0,-(sp)
	bsr     lab212
	move.l  (sp)+,d0
	lea     12(sp),sp
	movem.l (sp)+,d2-d7/a2-a6
	rts

lab13A  movem.l d2/d3/a2/a3,-(sp)
	moveq   #0,d2
	moveq   #0,d3
	jsr     -$120(a6)
	move.l  a0,a1
	move.l  a4,a0
	bsr.b   lab1A4
	move.l  d0,a2
	beq.b   lab186
	lea     8(a2),a1
	move.w  4(a2),d0
	move.l  a1,d1
lab15A  cmp.b   #$2E,(a1)+
	dbeq    d0,lab15A

	bne.b   lab174
	exg     d1,a1
	sub.l   a1,d1
	move.l  a2,d3
	move.l  a4,a0
	move.l  d1,d0
	bsr.b   lab1A4
	move.l  d0,a2
	beq.b   lab186
lab174  lea     8(a2),a0
	jsr     -$66(a6)
	cmp.w   4(a2),d1
	beq.b   lab188
	moveq   #$28,d2
	bra.b   lab188

lab186  moveq   #3,d2
lab188  tst.l   d2
	beq.b   lab198
	move.l  a4,a0
	move.l  a2,a1
	bsr.b   lab1D8
	move.l  a4,a0
	move.l  d3,a1
	bsr.b   lab1D8
lab198  move.l  a2,a1
	move.l  d3,d1
	move.l  d2,d0
	movem.l (sp)+,d2/d3/a2/a3
	rts

lab1A4  movem.l d0/a1,-(sp)
	addq.l  #8,d0
	addq.l  #1,d0
	jsr     -$72(a6)
	movem.l (sp)+,d0/a1
	beq.b   lab1D4
	move.l  a0,-(sp)
	clr.l   (a0)
	move.w  d0,4(a0)
	move.b  #2,6(a0)
	clr.b   8(a0,d0.l)
	addq.l  #8,a0
	jsr     -$10E(a6)
	move.l  (sp)+,a0
	move.b  d0,7(a0)
lab1D4  move.l  a0,d0
	rts

lab1D8  move.l  a1,d1
	beq.b   lab1F0
	moveq   #1,d0
	and.b   6(a1),d0
	bne.b   lab1F0
	move.w  4(a1),d0
	addq.l  #8,d0
	addq.l  #1,d0
	jsr     -$78(a6)
lab1F0  rts

lab1F2  move.l  a3,-(sp)
	move.l  $14(a2),a3
	move.l  a2,(a1)
	movem.l $FC(a3),d0/d1
	movem.l a0/a1,$FC(a3)
	movem.l d0/d1,4(a1)
	move.l  (sp)+,a3
	rts

lab212  move.l  (a0),a1
	move.l  $14(a1),a1
	movem.l 4(a0),d0/d1
	movem.l d0/d1,$FC(a1)
	rts

rexxlibTxt       dc.b    'rexxsyslib.library',0,0

	END
