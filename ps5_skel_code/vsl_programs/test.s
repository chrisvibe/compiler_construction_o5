.section .rodata
intout: .string "%ld"
strout: .string "%s"
errout: .string "Wrong number of arguments"
errprint: .string "Cant print this symbol"
errgen: .string "GENERIC ERROR!!!"
STR0: .string "Hello, world!"
STR1: .string "Goodbye, world!"
.section .data
_x: .zero 8
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
	call _hello
	jmp END
ABORT:
	movq $errout, %rdi
	call puts
END:
	movq %rax, %rdi
	call exit
.section .text
_hello:
	pushq %rbp
	movq %rsp, %rbp
	movq $STR0, %rax
	movq %rax, %rsi
	movq $strout, %rdi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $STR1, %rax
	movq %rax, %rsi
	movq $strout, %rdi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $8, %rax
	movq %rax, 0(%rbp)
	movq 0(%rbp), %rax
	movq %rax, %rsi
	movq $intout, %rdi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $1, %rax
	call exit
	popq %rbp
	movq %rsp, %rbp
