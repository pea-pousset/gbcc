;===============================================================================
; Interrupt and reset vectors
;===============================================================================

.org $00        ; RST $00
        RET

.org $08        ; RST $08
        RET

.org $10        ; RST $10
        RET

.org $18        ; RST $18
        RET

.org $20        ; RST $20
        RET

.org $28        ; RST $28
        RET

.org $30        ; RST $30
        RET

.org $38        ; RST $28
        RET

.org $40        ; VBLANK INTERRUPT
        RETI

.org $48        ; STATUS INTERRUPT
        RETI

.org $50        ; TIMER INTERRUPT
        RETI

.org $58        ; SERIAL INTERRUPT
        RETI

.org $60        ; JOYPAD INTERRUPT
        RETI
        

;===============================================================================
; Arithmetic ops
;===============================================================================
.org $68
.global ___mul_8_
.global ___div_u8_

; 8 bit multiplication
; HL = H * E
;-------------------------------------------------------------------------------
___mul_8_:
        LD      D,  0
        LD      L,  D
        LD      B,  8
___mul_8_loop:
        ADD     HL, HL
        JR      NC, ___mul_8_skip_add
        ADD     HL, DE
___mul_8_skip_add:
        DEC     B
        JR      NZ, ___mul_8_loop
        RET

; 8 bit unsigned division and modulo
; D = D/E, A = D%E
;------------------------------------------------------------------------------- 
___div_u8_:
        XOR     A
        LD      B, 8
___div_u8_loop:
        SLA     D
        RLA
        CP      E
        JR      C, ___div_u8_skip_sub
        SUB     E
        INC     D
___div_u8_skip_sub:
        DEC     B
        JR      NZ, ___div_u8_loop
        RET

;===============================================================================
; Header
;===============================================================================
.org    $100
; Startup code
        NOP
        JP      ___main

; Nintendo logo
        .byte   $CE, $ED, $66, $66, $CC, $0D, $00, $0B
        .byte   $03, $73, $00, $83, $00, $0C, $00, $0D
        .byte   $00, $08, $11, $1F, $88, $89, $00, $0E
        .byte   $DC, $CC, $6E, $E6, $DD, $DD, $D9, $99
        .byte   $BB, $BB, $67, $63, $6E, $0E, $EC, $CC
        .byte   $DD, $DC, $99, $9F, $BB, $B9, $33, $3E
        
; Title
        .byte   'O', 'U', 'T', '.', 'G', 'B', ' ', ' ', ' ', ' ', ' '
        
; Manufacturer code
        .byte   $BA, $AD, $F0, $0D
        
; Gameboy Color compatibility
        .byte   0
        
; New licensee code
        .byte   ' ', ' '
        
; Super Gameboy compatibility
        .byte   0
        
; Cartridge Type
        .byte   0

; Rom size
        .byte   0

; Ram size
        .byte   0
        
; Destination code
        .byte   1
        
; Old licensee code
        .byte   $33
        
; Rom version
        .byte   0
        
; Header checksum
        .byte   0
        
; Global checksum
        .word   0

;===============================================================================
; MAIN
;=============================================================================== 
.org    $150
___main:
        DI
        LD      SP, $FFFE

        CALL    init_nums

        CALL    main        

_inf_loop:
        HALT
        NOP
        JR      _inf_loop
        



.org    $D000           ; WRAM BANK 1
cur_y:  .word

.org    $4000           ; ROM BANK 1
.global print_a

init_nums:
; Turn off the screen
ldh a,  [$44]           ; LCDC Y
cp      $90
jr      nz, init_nums
xor     a
ldh     [$40], a        ; LCD_CTRL

; Reset scrolling
ldh     [$42], a        ; SCY
ldh     [$43], a        ; SCX

ld      hl, cur_y
ldi     [hl], a
ld      [hl], a

; Load tiles
ld      hl, num_tiles
ld      de, $8000       ; VRAM
ld      b,  16          ; 17 tiles
__loop_1:
ldi     a,  [hl]
ld      [de], a
inc     de
dec     c
jr      nz, __loop_1
ld      c,  17
dec     b
jr      nz,  __loop_1

; Clear background
ld      hl, $9800
ld      b,  $20
ld      c,  $20
ld      a,  16          ; Empty tile
init_nums_loop_2:
ldi     [hl], a
dec     c
jr      nz, init_nums_loop_2
ld      c, $20
dec     b
jr      nz, init_nums_loop_2

ld      a, $91
ldh     [$40], a
ret


print_a:
ld      c, a
ld      b, 0
jr      print_hl_wait_vblank
print_hl:
ld      b, h
ld      c, l
print_hl_wait_vblank:
ldh     a, [$44]    ; LCD_Y
cp      $90
jr      nz, print_hl_wait_vblank

; Value of cur_y in DE
ld      hl, cur_y
ldi     a, [hl]
ld      e, a
ld      a, [hl]
ld      d, a

ld      hl, $9800
add     hl, de

; print b
ld      a, b
swap    a
and     $0F
ldi     [hl], a
ld      a, b
and     $0F
ldi     [hl], a
; print c
ld      a, c
swap    a
and     $0F
ldi     [hl], a
ld      a, c
and     $0F
ld      [hl], a

ld      a, e
add     a, $20
ld      e, a
jr      nc, __skip
inc     d
__skip:
ld      hl, cur_y
ld      [hl], e
inc     hl
ld      [hl], d
ret

num_tiles:
.byte $00,$00,$3C,$3C,$66,$66,$6E,$6E
.byte $7E,$7E,$76,$76,$66,$66,$3C,$3C
.byte $00,$00,$18,$18,$78,$78,$18,$18
.byte $18,$18,$18,$18,$18,$18,$7E,$7E
.byte $00,$00,$3C,$3C,$66,$66,$06,$06
.byte $1C,$1C,$30,$30,$66,$66,$7E,$7E
.byte $00,$00,$3C,$3C,$66,$66,$06,$06
.byte $1C,$1C,$06,$06,$66,$66,$3C,$3C
.byte $00,$00,$0E,$0E,$1E,$1E,$36,$36
.byte $66,$66,$7F,$7F,$06,$06,$06,$06
.byte $00,$00,$7E,$7E,$60,$60,$7C,$7C
.byte $06,$06,$06,$06,$66,$66,$3C,$3C
.byte $00,$00,$1C,$1C,$30,$30,$60,$60
.byte $7C,$7C,$66,$66,$66,$66,$3C,$3C
.byte $00,$00,$7E,$7E,$66,$66,$06,$06
.byte $0C,$0C,$18,$18,$30,$30,$30,$30
.byte $00,$00,$3C,$3C,$66,$66,$66,$66
.byte $3C,$3C,$66,$66,$66,$66,$3C,$3C
.byte $00,$00,$3C,$3C,$66,$66,$66,$66
.byte $3E,$3E,$06,$06,$0C,$0C,$38,$38
.byte $00,$00,$18,$18,$3C,$3C,$66,$66
.byte $66,$66,$7E,$7E,$66,$66,$66,$66
.byte $00,$00,$7E,$7E,$33,$33,$33,$33
.byte $3E,$3E,$33,$33,$33,$33,$7E,$7E
.byte $00,$00,$1E,$1E,$33,$33,$60,$60
.byte $60,$60,$60,$60,$33,$33,$1E,$1E
.byte $00,$00,$7E,$7E,$36,$36,$33,$33
.byte $33,$33,$33,$33,$36,$36,$7E,$7E
.byte $00,$00,$7F,$7F,$31,$31,$34,$34
.byte $3C,$3C,$34,$34,$31,$31,$7F,$7F
.byte $00,$00,$7F,$7F,$31,$31,$34,$34
.byte $3C,$3C,$34,$34,$30,$30,$78,$78
.byte $00,$00,$00,$00,$00,$00,$00,$00
.byte $00,$00,$00,$00,$00,$00,$00,$00






