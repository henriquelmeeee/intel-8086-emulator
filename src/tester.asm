[org 0x7c00]
[bits 16]

main:
  inc ax
  cmp al, 10
  je end
  jmp main
end:
  hlt
