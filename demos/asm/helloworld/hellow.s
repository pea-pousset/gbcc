.org $100   ; entry point
    jp start
    nop

; rom header not required =)

.org $150
start:
    di
    ld      sp, $FFFF
wait_vblank:
    ldh     a, [$44]        ; LCD Y
    cp      144
    jr      c, wait_vblank  ; loop while LCD Y < 144
    xor     a
    ldh     [$40], a        ; LCD Control: off

    ld      hl, sprites     ; sprites.s
    ld      de, $8000
    ld      bc,  4096       ; 256 tiles * 16 bytes
    call    memcpy
    call    clear_screen

    ; init cursor position
    xor     a
    ld      hl, cx
    ldi     [hl], a
    ld      [hl], a

    ; turn on LCD, bg only, tiles data at $8000, bg map at $9800
    ld      a, $91
    ldh     [$40], a

    ld      de, hellowstr
    call    print

inf_loop:
    halt
    nop
    jr      inf_loop

hellowstr:
.byte 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '!', '!', 0


;-------------------------------------------------------------------------------
; memcpy
; hl: source, de: destination, bc: count
;-------------------------------------------------------------------------------
memcpy:
    inc     b
    inc     c
    jr      _memcpy_skip
_memcpy_loop:
    ldi     a, [hl]
    ld      [de],a
    inc     de
_memcpy_skip:
    dec     c
    jr      nz, _memcpy_loop
    dec     b
    jr      nz, _memcpy_loop
    ret

;-------------------------------------------------------------------------------
; clear_screen
;-------------------------------------------------------------------------------
clear_screen:
    ld      a, ' '
    ld      hl, $9800
    ld      c, 32
_clear_screen_loop:
    ldi     [hl], a
    dec     b
    jr      nz, _clear_screen_loop
    dec     c
    ld      b, 32
    jr      nz, _clear_screen_loop
    ret

print:
    ld      a, [de]
    or      a
    ret     z
    ld      c, a
    call    putchar
    inc     de
    jr      print

;-------------------------------------------------------------------------------
; putchar
; c: character
; modified: a
;-------------------------------------------------------------------------------
putchar:
    ldh     a, [$44]        ; LCD Y
    cp      144
    jr      c, putchar        ; loop while LCD Y < 144

    ld      hl, $9800
    ld      a, [cy]
    or      a
    jr      z, _putchar_skip_cy
    push    de
    ld      de, 32
_putchar_inc_y:
    add     hl, de
    dec     a
    jr      nz, _putchar_inc_y
    pop     de
_putchar_skip_cy:
    ld      a, [cx]
    or      a
    jr      z, _putchar_skip_cx
_putchar_inc_x:
    inc     hl
    dec     a
    jr      nz, _putchar_inc_x

_putchar_skip_cx:
    ld      [hl], c
    ld      a, [cx]
    inc     a
    cp      20
    jr      z, _putchar_inc_cy
    ld      [cx], a
    ret
_putchar_inc_cy:
    xor     a
    ld      [cx], a
    ld      a, [cy]
    inc     a
    ld      [cy], a

    ret

.org $C000 ; WRAM 0
cx: .byte
cy: .byte
