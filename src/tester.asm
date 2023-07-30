[bits 16]
[org 0x7c00]

main:
  ;mov ah, 0x0e
  ;mov al, 'O'
  mov ah, 0x0e
  mov al, 'h'
  int 0x10
  call teste
  nop

teste:
  mov ah, 0x0e
  mov al, 'e'
  int 0x10
  mov al, 'l'
  int 0x10
  mov al, 'l'
  int 0x10
  mov al, 'o'
  int 0x10
  mov al, '!'
  int 0x10
  mov dh, 2 ; linha 1
  mov bl, 2
  mov bh, 2
  mov ah, 0x02
  int 0x10
  jmp main

times 512 - ($ - $$)  db 0
dw 0xAA55
