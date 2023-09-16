[org 0x7c00]
[bits 16]

main:
  mov ax, 0x7c00
  mov bx, ax
  mov [bx], 'a'
  jmp main
