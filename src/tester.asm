[bits 16]
[org 0x7c00]

main:
  mov ah, 0x0e
  mov al, 'O'
  int 0x10
  int 3

times 512 - ($ - $$)  db 0
dw 0xAA55
