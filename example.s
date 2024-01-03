section .data
    x db 200    ; Define a byte variable 'x' with initial value 200
    format db "%d", 10  ; Define the format string for printf

section .text
    global main
    extern printf

main:
    ; Print the value of 'x'
    movzx eax, byte [x]    ; Move the value of 'x' into the eax register
    push eax               ; Push the value onto the stack for printf

    
    push format            ; Push the format string onto the stack
    call printf            ; Call the printf function
    add esp, 8             ; Adjust the stack pointer after the function call

    ; Exit the program
    mov eax, 1             ; syscall number for exit
    xor ebx, ebx           ; status 0
    int 0x80               ; make syscall
   