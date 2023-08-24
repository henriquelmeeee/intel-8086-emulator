[bits 16]
[org 0x7c00]

main:
  mov ah, 0x0e
  mov bx, 0
  mov al, 'H'
  int 0x10
  mov al, 'e'
  int 0x10
  mov al, 'l'
  int 0x10
  mov al, 'l'
  int 0x10
  mov al, 'o'
  int 0x10
  mov al, ''
  int 0x10
  mov al, 'W'
  int 0x10
  mov al, 'o'
  int 0x10
  mov al, 'r'
  int 0x10
  mov al, 'l'
  int 0x10
  mov al, 'd'
  int 0x10
  mov al, '!'
  int 0x10
  call user_input

user_input:
  mov ah, 0
  int 0x16
  mov ah, 0x0e
  int 0x10
  jmp user_input

db "Oi, tudo bem?"
