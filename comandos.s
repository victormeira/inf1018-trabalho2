
.text
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $1, %eax
	leave
	ret

