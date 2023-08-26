[bits 16]
[org 0x7c00]

main:
  mov si, prompt
  mov di, 0
  call print
  hlt


print:
  mov ax, si
  add ax, di
  mov si, ax

  mov al, byte [si]

  cmp al, 0
  je print_end
  
  mov ah, 0x0e
  int 0x10
  add di, 1
  jmp print

  print_end:
    ret


prompt db "Shell> "
