
[bits 16]
[org 0x7C00]

; Definir portos de E/S ATA
ATA_DATA_PORT    equ 0x1F0
ATA_SECTOR_COUNT equ 0x1F2
ATA_LBA_LOW      equ 0x1F3
ATA_LBA_MID      equ 0x1F4
ATA_LBA_HIGH     equ 0x1F5
ATA_DRIVE_SELECT equ 0x1F6
ATA_COMMAND_PORT equ 0x1F7
ATA_STATUS_PORT  equ 0x1F7

; Definir comando ATA
ATA_READ_CMD     equ 0x20

destino equ 0

; Início
start:
    ; Esperar que o controlador ATA esteja pronto
    call wait_for_controller

    ; Selecionar drive e modo LBA
    mov dx, ATA_DRIVE_SELECT
    mov al, 0xE0 ; 0xE0 para o drive primário, 0x00 para o setor 0
    out dx, al

    ; Configurar contagem de setores para ler
    mov dx, ATA_SECTOR_COUNT
    mov al, 1 ; Ler 1 setor
    out dx, al

    ; Configurar LBA (setor 0)
    mov dx, ATA_LBA_LOW
    xor al, al
    out dx, al
    mov dx, ATA_LBA_MID
    out dx, al
    mov dx, ATA_LBA_HIGH
    out dx, al

    ; Enviar comando de leitura
    mov dx, ATA_COMMAND_PORT
    mov al, ATA_READ_CMD
    out dx, al

    ; Esperar que o controlador esteja pronto para enviar dados
    call wait_for_controller

    ; Ler dados do setor
    mov cx, 256 ; 512 bytes (256 words)
    mov bx, destino ; Endereço de destino dos dados
    mov dx, ATA_DATA_PORT
read_loop:
    in ax, dx ; Ler palavra do porto de dados em ax
    loop read_loop

    ; Encerrar (em um sistema real, aqui iria o código para encerrar ou reiniciar)
    hang:
        jmp hang

; Esperar que o controlador ATA esteja pronto
wait_for_controller:
    mov dx, ATA_STATUS_PORT
wait_loop:
    in al, dx
    test al, 0x80 ; Verificar se o bit BUSY está ativo
    jnz wait_loop
    test al, 0x08 ; Verificar se o bit DRQ está ativo
    jz wait_loop
    ret

; Endereço de destino para os dados lidos
destino:
    times 512 db 0

; Preencher o restante do bootloader
times 510-($-$$) db 0
dw 0xAA55
