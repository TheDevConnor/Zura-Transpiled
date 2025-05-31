# ╔═════════════════════════════════╗
# ║   Zura Syntax by TheDevConnor   ║
# ║   assembly by Soviet Pancakes   ║
# ╚═════════════════════════════════╝
# v0.1.45
# What's New: Dereferencing structs

# Everything beyond this point was generated automatically by the Zura compiler.
.att_syntax
.bss
.Largc:
  .type .Largc, @object
  .zero 8
  .size .Largc, 8

.Largv:
  .type .Largv, @object
  .zero 8
  .size .Largv, 8

.text
.globl _start
_start:
  .cfi_startproc
  movq (%rsp), %rax
  movq %rax, .Largc(%rip)
  leaq 8(%rsp), %rax
  movq %rax, .Largv(%rip)
  call main
  xorq %rdi, %rdi
  movq $60, %rax
  syscall
  .cfi_endproc
.size _start, .-_start

.type usr_readFile, @function
.globl usr_readFile

usr_readFile:
	.cfi_startproc
	endbr64
	pushq %rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq %rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq %rdi, -8(%rbp)
	movq $2048, -16(%rbp)
	movq -16(%rbp), %rsi
	movq $9, %rax
	movq $0, %rdi
	movq $3, %rdx
	movq $34, %r10
	movq $-1, %r8
	movq $0, %r9
	syscall # SYS_MMAP
	movq %rax, -24(%rbp)
	movq -8(%rbp), %rdi
	movq $2, %rsi
	movq $388, %rdx
	movq $2, %rax
	syscall # SYS_OPEN
	movq %rax, -32(%rbp)
	movq $0, %rbx
	movq -32(%rbp), %rax
	cmpq %rbx, %rax
	jge .Lifend0
	leaq string0(%rip), %rsi
	movq $28, %rdx
	movq $1, %rdi
	movq $1, %rax
	syscall # SYS_WRITE
	movq -8(%rbp), %rsi
	movq $1, %rdi
	movq $1, %rax
	syscall # SYS_WRITE
	leaq string1(%rip), %rsi
	movq $1, %rdx
	movq $1, %rdi
	movq $1, %rax
	syscall # SYS_WRITE
	movq -24(%rbp), %rdi
	movq -16(%rbp), %rsi
	movq $11, %rax
	syscall # SYS_MUNMAP
	xorq %rax, %rax
	popq %rbp
	.cfi_def_cfa 7, 8
	ret # 
	jmp .Lifend0
	
.Lifend0:
	movq -24(%rbp), %rsi
	movq -16(%rbp), %rdx
	movq -32(%rbp), %rdi
	xorq %rax, %rax
	syscall # SYS_READ
	movq -32(%rbp), %rdi
	movq $3, %rax
	syscall # SYS_CLOSE
	movq -24(%rbp), %rax
	popq %rbp
	.cfi_def_cfa 7, 8
	ret # 
	.cfi_endproc
.size usr_readFile, .-usr_readFile
	
.type main, @function
.globl main

main:
	.cfi_startproc
	endbr64
	pushq %rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq %rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq .Largv(%rip), %r13
	movq %r13, -8(%rbp)
	movq .Largc(%rip), %r13
	movq %r13, -16(%rbp)
	movq $2, %rbx
	movq -16(%rbp), %rax
	cmpq %rbx, %rax
	jge .Lifend1
	leaq string2(%rip), %rsi
	movq $58, %rdx
	movq $1, %rdi
	movq $1, %rax
	syscall # SYS_WRITE
	leaq string3(%rip), %rsi
	movq $1, %rdx
	movq $1, %rdi
	movq $1, %rax
	syscall # SYS_WRITE
	movl $enum_ERRORS_NoArgumentsProvided, %edi
	movq $60, %rax
	syscall # SYS_EXIT
	jmp .Lifend1
	
.Lifend1:
	movq $0, -24(%rbp)
	movq $0, -32(%rbp)
	
loop_pre0:
	movq -16(%rbp), %rbx
	movq -32(%rbp), %rax
	cmpq %rbx, %rax
	jge loop_post0
	movq -8(%rbp), %rcx
	movq -32(%rbp), %rdi
	movq (%rcx, %rdi, 8), %rdi
	leaq string4(%rip), %rsi
	subq $32, %rsp
	call native_strcmp
	addq $32, %rsp
	movzxb %al, %rax
	testq %rax, %rax
	jz .Lifend3
	movq $1, -40(%rbp)
	movq -16(%rbp), %rax
	movq -40(%rbp), %rbx
	sub %rbx, %rax
	pushq %rax
	movq -32(%rbp), %rax
	popq %rbx
	cmpq %rbx, %rax
	jne .Lifend4
	leaq string5(%rip), %rsi
	movq $28, %rdx
	movq $1, %rdi
	movq $1, %rax
	syscall # SYS_WRITE
	leaq string6(%rip), %rsi
	movq $1, %rdx
	movq $1, %rdi
	movq $1, %rax
	syscall # SYS_WRITE
	movl $enum_ERRORS_NoPathProvided, %edi
	movq $60, %rax
	syscall # SYS_EXIT
	jmp .Lifend4
	
.Lifend4:
	incq -32(%rbp)
	movq -8(%rbp), %rcx
	movq -32(%rbp), %rdi
	movq (%rcx, %rdi, 8), %r13
	movq %r13, -24(%rbp)
	jmp .Lifend3
	
.Lifend3:
	incq -32(%rbp)
	jmp loop_pre0
	
loop_post0:
	movq $0, %rbx
	movq -24(%rbp), %rax
	cmpq %rbx, %rax
	jne .Lifend5
	movl $enum_ERRORS_NoActionProvided, %edi
	movq $60, %rax
	syscall # SYS_EXIT
	jmp .Lifend5
	
.Lifend5:
	subq $24, %rsp
	movq -24(%rbp), %rdi
	call usr_readFile
	addq $24, %rsp
	movq %rax, -32(%rbp)
	movq -32(%rbp), %rsi
	movq $1, %rdi
	movq $1, %rax
	syscall # SYS_WRITE
	movq -32(%rbp), %rdi
	movq $2048, %rsi
	movq $11, %rax
	syscall # SYS_MUNMAP
	xorq %rdi, %rdi
	movq $60, %rax
	syscall # SYS_EXIT
	popq %rbp
	.cfi_def_cfa %rsp, 8
	ret # main
	.cfi_endproc
.size main, .-main
	
# zura functions
.type native_strcmp, @function
native_strcmp:
.Lstrcmp_loop:
    movzbq (%rdi), %rax      # Load *rdi -> rax (zero-extend)
    movzbq (%rsi), %rcx      # Load *rsi -> rcx (zero-extend)
    cmp %rax, %rcx           # Compare characters
    jne .Lstrcmp_diff        # If different, return difference
    test %al, %al            # Check for null terminator
    je .Lstrcmp_equal        # Both null → equal
    inc %rdi
    inc %rsi
    jmp .Lstrcmp_loop
.Lstrcmp_diff:
    xorq %rax, %rax          # return false (they are not equal)
    ret
.Lstrcmp_equal:
    movq $1, %rax            # Return true (they are equal)
    ret
.size native_strcmp, .-native_strcmp

# readonly data section - contains constant strings and floats (for now)
.section .rodata
.set enum_ERRORS_InvlaidArgument, 0
.set enum_ERRORS_OutputError, 1
.set enum_ERRORS_NoArgumentsProvided, 2
.set enum_ERRORS_NoPathProvided, 3
.set enum_ERRORS_NoActionProvided, 4

string0:
	.asciz "ERROR: Could not open file: "
	
string1:
	.asciz "\n"
	
string2:
	.asciz "No arguments provided. Please provide a string to reverse."
	
string3:
	.asciz "\n"
	
string4:
	.asciz "build"
	
string5:
	.asciz "ERROR: No filepath to build."
	
string6:
	.asciz "\n"
	