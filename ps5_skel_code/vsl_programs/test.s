.section .rodata
intout: .string "%ld"
strout: .string "%s"
errout: .string "Wrong number of arguments"
errprint: .string "Cant print this symbol"
errgen: .string "GENERIC ERROR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
STR0: .string "Hello, world!"
STR1: .string "Goodbye, world!"
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
.section .data
_hello:
	pushq %rbp
	movq %rsp, %rbp
	movq $strout, %rdi
	movq $STR0, %rsi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $strout, %rdi
	movq $STR1, %rsi
	call printf
	movq $'\n', %rdi
	call putchar
	movq $0, %rax
	call exit
