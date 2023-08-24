[bits 16]
[org 0x7c00]

main:
  mov ah, 0x0e
  mov bx, 0
  mov al, 'b'
  int 0x10
  jmp main
