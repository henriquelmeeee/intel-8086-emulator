[bits 16]
[org 0x7c00]

main:
  ;mov ah, 0x0e
  ;mov al, 'O'
  call teste

teste:
  add al, 1
  jb finish
  nop
  jmp teste

finish:
  mov ah, 0x0e
  mov al, 'a'
  int 0x10
  jmp finish

times 512 - ($ - $$)  db 0
dw 0xAA55
