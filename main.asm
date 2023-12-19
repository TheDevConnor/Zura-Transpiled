section .data
    fmt db "%d + %d = %d", 10, 0

section .text
    global main
    extern printf

main:
    ; Prologue
    push ebp
    mov ebp, esp

    ; Local variable declarations
    mov byte [ebp-4], 10    ; a: i8 = 10
    mov byte [ebp-8], 20    ; b: i8 = 20

    ; Call printf
    mov eax, [ebp-4]        ; Load a
    mov ebx, [ebp-8]        ; Load b
    add eax, ebx            ; Add a and b
    push eax                ; Push the result
    push dword [ebp-8]      ; Push b
    push dword [ebp-4]      ; Push a
    push dword fmt          ; Push the format string
    call printf             ; Call printf
    add esp, 16             ; Clean up the stack

    ; Return 0
    mov eax, 0

    ; Epilogue
    pop ebp
    ret

    ; compile command:
    ; nasm -f elf32 main.asm -o main.o
    ; gcc -m32 -o add add.o
