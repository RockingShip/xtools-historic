__START(argc, argv)
int argc, *argv;
{
 	/* Load the special purpose registers */
	/* r15 REG_SP  initial sp */
	/* r14 REG_AP  frame pointer*/
	/* r13 REG_BPW constant BPW */
	/* r12 REG_4   constant 2*BPW */
	/* r11 REG_1   constant 1 */
	/* R10 REG_0   constant 0 */
#asm
	lda	r13,2
	lda	r12,4
	lda	r11,1
	lda	r10,0
#endasm

	main (argc, argv);
	exit (0);
}
