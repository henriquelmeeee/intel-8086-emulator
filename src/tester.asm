[bits 16]
[org 0x7c00]

main:
  ;mov ah, 0x0e
  ;mov al, 'O'
  mov ah, 0x0e
  mov al, 'v'
  int 0x10
  nop

teste:
  mov ah, 0x0e
  mov al, 'o'
  int 0x10
  mov al, 'u'
  int 0x10
  mov al, ' '
  int 0x10
  mov al, 'p'
  int 0x10
  mov al, 'r'
  int 0x10
  mov al, 'a'
  int 0x10
  mov al, ' '
  int 0x10
  mov al, 'o'
  int 0x10
  mov al, 'u'
  int 0x10
  mov al, 't'
  int 0x10
  mov al, 'r'
  int 0x10
  mov al, 'a'
  int 0x10
  mov al, ' '
  int 0x10
  mov al, 'l'
  int 0x10
  mov al, 'i'
  int 0x10
  mov al, 'n'
  int 0x10
  mov al, 'h'
  int 0x10
  mov al, 'a'
  int 0x10
  mov al, '!'
  int 0x10
  ;mov dh, 1 ; linha 1
  ;mov bl, 2
  ;mov bh, 2
  ;mov ah, 0x02
  ;int 0x10
  jmp main

times 512 - ($ - $$)  db 0
dw 0xAA55
