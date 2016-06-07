.text
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $257, %eax
	movl $1, %ecx
	addl %ecx, %eax
	subq $16, %rsp
	addl %ecx, %eax
	movl %eax,-3(%rbp)
	movl -4(%rbp), %eax
	leave
	ret

