.text
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $1, %eax
	movl $2, %ecx
	addl %ecx, %eax
	subq $16, %rsp
	movl %eax, -4(%rbp)
	movl -4(%rbp), %eax
	movl -4(%rbp), %ecx
	addl %ecx, %eax
	movl %eax,-8(%rbp)
	movl -8(%rbp), %eax
	leave
	ret

