[bits 16]
[org 0x7c00]

main:
  ;mov ax, 0x7c0
  ;mov ds, ax
  mov si, prompt
  call print
  main_loop:
    call wait_user_input
    jmp main_loop
  hlt

print:
  mov ah, 0x0e
  lodsb
  cmp al, 0
  jz print_end
  int 0x10
  jmp print
  print_end:
    ret

wait_user_input:
  mov ah, 0
  int 0x16
  cmp al, 0
  je just_exit
  mov ah, 0x0e
  int 0x10
  jmp wait_user_input

just_exit:
  hlt

prompt db "Shell> ", 0
