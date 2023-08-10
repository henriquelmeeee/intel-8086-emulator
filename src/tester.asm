[bits 16]
[org 0x7c00]

main:
  ;mov ah, 0x0e
  ;mov al, 'O'
  call teste

teste:
  ;mov bx, 1
  ;add byte [bx + si], al
  ;nop
  ;jmp teste
  jmp finish

finish:
  mov ah, 0x0e
  mov al, 'a'
  int 0x10
  mov ax, 220
  mov sp, 0
  call teste_overflow

teste_overflow:
  mov al, 'b'
  int 0x10
  jmp teste_overflow

times 512 - ($ - $$)  db 0
dw 0xAA55
