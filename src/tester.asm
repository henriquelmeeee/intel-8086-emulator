[bits 16]
[org 0x7c00]

main:
  ;mov ax, 0x7c0
  ;mov ds, ax
  mov bx, 5
  mov [bx], sp
  add bl, bl
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
  je print_end
  int 0x10
  jmp print
  print_end:
    ret

line_break:
  mov ah, 0x0e
  mov al, 0x0d
  int 0x10

  mov al, 0x0a
  int 0x10
  mov si, prompt
  call print
  ret
  hlt

wait_user_input:
  mov ah, 0
  int 0x16
  cmp al, 0x0
  je just_exit
  cmp al, 0x0a
  je line_break_and_ret
  __ret:
    mov ah, 0x0e
    int 0x10
    jmp wait_user_input

  line_break_and_ret:
    call line_break
    jmp __ret

just_exit:
  hlt

prompt db "Shell> ",0
