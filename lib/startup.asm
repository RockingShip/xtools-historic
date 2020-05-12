#define REG_SP		r15
#define REG_AP		r14
#define REG_BPW		r13
#define REG_4		r12
#define REG_1		r11
#define REG_0		r10

___START::
	lda	REG_SP,0
	lda	REG_BPW,2
	lda	REG_4,4
	lda	REG_1,1
	lda	REG_0,0

	jsb	__main
	jsb	_exit

sav_svc:
	.dsw	1
_SVC::
	stw	r1,sav_svc
	ldw	r1,4(r15)
	stw	r1,lbl+1
lbl:	svc	0
	ldw	r1,sav_svc
	rsb

	.end
