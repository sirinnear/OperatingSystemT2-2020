bits 16
org 0x7c00

start:

boot:
            mov si,message
            mov ah,0Eh
lp1         lodsb
            cmp al, 0
            jne printpls
            cli
            hlt
printpls    int 10h
            jmp lp1

data:
            message db 'Possawat', 0
times 510- ($-$$) db 0
dw 0xAA55