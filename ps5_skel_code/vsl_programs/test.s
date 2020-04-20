.section .rodata
intout: .string "%ld"
strout: .string "%s"
errout: .string "Wrong number of arguments"
errprint: .string "Cant print this symbol"
errgen: .string "GENERIC ERROR!!!"
.section .data
.globl main
.section .text
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $1, %rdi
	cmpq $0, %rdi
	jne ABORT
	cmpq $0, %rdi
	jz SKIP_ARGS
	movq %rdi, %rcx
	addq $0, %rsi
PARSE_ARGV:
	pushq %rcx
	pushq %rsi
	movq (%rsi), %rdi
	movq $0, %rsi
	movq $10, %rdx
	call strtol
	popq %rsi
	popq %rcx
	pushq %rax
	subq $8, %rsi
	loop PARSE_ARGV
SKIP_ARGS:
	call _main
	jmp END
ABORT:
	movq $errout, %rdi
	call puts
END:
	movq %rax, %rdi
	call exit
.section .text
_main:
	pushq %rbp
	movq %rsp, %rbp
	call _increment
	movq %rax, -8(%rbp)
	movq -8(%rbp), %rax
	movq %rax, %rsi
	movq $intout, %rdi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $0, %rax
	popq %rbp
	retq
.section .text
_increment:
	pushq %rbp
	movq %rsp, %rbp
	movq $9, %rax
	popq %rbp
	retq
