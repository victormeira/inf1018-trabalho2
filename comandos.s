
.text
.globl main
main:
	imull $127, %ecx
	imull $128, %ecx
	subq $16, %rsp
	ret 

