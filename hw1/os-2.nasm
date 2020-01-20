bits 16
org 0x7c00

start:
            mov si, message
            call print1
boot:
            call scan
            mov si, nsp
            call print1
            mov si, typed
            call print1
            mov si, name
            call print1
            cli
            hlt
scan:
            mov di, name
keep        mov ah, 0h
            int 16h
            mov ah, 0Eh
            int 10h
            stosb
            cmp al, 0dh
            jne keep
            mov di, si
            ret
print1:
            mov ah, 0Eh
again       lodsb
            cmp al, 0
            jne printpls
            ret
printpls    int 10h
            jmp again
data:
    message db 'What is your name? ', 0
    typed db 'Hello, ', 0
    nsp db '', 10,13,0
.bss:
    name: resb 64
times 510- ($-$$) db 0
dw 0xAA55