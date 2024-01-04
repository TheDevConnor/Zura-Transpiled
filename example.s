section .data
    x dw 200          ; Define a word variable 'x' (i16) with initial value 200
    format db "%d", 10  ; Define the format string for printf

section .text
    extern printf, exit

global main
main:
    ; Set up stack frame
    push rbp
    mov rbp, rsp

    ; Move 16-bit 'x' to 64-bit register rax to prepare for printf
    movsx rax, word [x]    ; Move 16-bit 'x' to 64-bit register rax

    ; Call printf
    lea rdi, [format]      ; Load address of format string into rdi (1st arg for printf)
    mov rsi, rax           ; Move value of 'x' (now in rax) to rsi (2nd arg for printf)
    xor eax, eax           ; Clear eax to indicate printf uses vector registers for arguments
    call printf            ; Call printf function

    ; Exit the program
    xor edi, edi           ; Clear edi for exit status 0
    call exit              ; Call exit function

    ; Clean up and return
    mov rsp, rbp
    pop rbp
    ret