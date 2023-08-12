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
  mov ah, 2
  mov al, 1
  mov ch, 0
  mov cl, 0
  mov dh, 0
  mov dl, 0
  mov bx, 0x0
  int 0x13
  jmp 0x0

times 512 - ($ - $$)  db 0
dw 0xAA55
