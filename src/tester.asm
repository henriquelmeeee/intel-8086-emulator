[bits 16]
[org 0x7c00]

main:
  mov si, prompt
  call print
  hlt


print:
  mov al, byte [si]
  
  mov ah, 0x0e
  int 0x10
  jmp print
  lodsb


prompt db "Shell> "
