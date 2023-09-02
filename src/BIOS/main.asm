[org 0xf0000]
[bits 16]

; BIOS interrupts are hard-coded for simplicity

; TODO mov al, 0x01 = update cursor after writing; an feature of INT 0X10 that we do not handle at now

main:
  mov sp, 0x7bff
  mov bp, sp
  mov si, bios_post_msg
  mov ah, 0x13
  int 0x10
  jmp cmos_get_info

cmos_get_info:
  ;mov al, 0
  ;out 0x70, al
  ;in al, 0x71
  ; ... todo continuar
  jmp fill_bda

fill_bda:
  mov di, 0x0400
  mov si, BIOS_DATA_AREA
  
  mov ax, WORD [si]
  loop:
    mov WORD [di], ax
    add di, 2
    mov ax, WORD [si]
    cmp ax, 256
    je end
  end:
    jmp 0x7c00


section .data
bios_post_msg db "BIOS: Post in progress", 0
BIOS_DATA_AREA:
  keyboard_status_flags1 db 0
  keyboard_status_flags2 db 0
  keyboard_alt_keypad db 0
  keyboard_head dw 0
  keyboard_tail dw 0
  keyboard_buffer dw 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  floppy_recalibrated dw 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  floppy_motor_status db 0
  floppy_motor_counter db 0
  floppy_status db 0
  floppy_controller_status db 0
  floppy_return_status db 0,0,0,0,0,0,0
  display_mode db 0
  text_columns dw 0
  video_page_size dw 0
  video_page_start dw 16
  cursor_pos db 0,0,0,0,0,0
  cursor_type db 0
  rows db 0
  char_height dw 0
  video_control_states db 0
  switch_settings db 0
  drive_data db 0,0,0,0
  drive_current db 0
  keyboard_flags db 0
  alt_num_pad_work_area db 0
  ; ...

