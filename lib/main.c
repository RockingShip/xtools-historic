__START(int argc, int *argv)
{
 	/* Load the special purpose registers */
	/* r15 REG_SP  initial sp */
	/* r14 REG_AP  frame pointer*/
	/* r13 REG_BPW constant BPW */
	/* r12 REG_4   constant 2*BPW */
	/* r11 REG_1   constant 1 */
	/* R10 REG_0   constant 0 */

	asm("lda r13,2");
	asm("lda r12,4");
	asm("lda r11,1");
	asm("lda r10,0");

	main (argc, argv);
	exit (0);
}
