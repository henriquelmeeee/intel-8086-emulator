[org 0x7c00]
[bits 16]

main:
  mov bl, 1
  mov al, 1
  _add:
    add bl, al
    jmp _add
