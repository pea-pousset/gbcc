SECTION "___crt0___", ROM0[$0000]

;===============================================================================
; Interrupt and reset vectors
;===============================================================================

; RST VECTOR $0000
        RET
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; RST VECTOR $0008
        RET
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; RST VECTOR $0010
        RET
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; RST VECTOR $0018
        RET
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; RST VECTOR $0010
        RET
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; RST VECTOR $0018
        RET
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; RST VECTOR $0020
        RET
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; RST VECTOR $0028
        RET
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; VBLANK INTERRUPT  $0040
SECTION "___VBLANK_INT___",ROM0[$0040]
        RETI
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; STATUS INTERRUPT  $0048
        RETI
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; TIMER INTERRUPT $0050
        RETI
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; SERIAL INTERRUPT  $0058
        RETI
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

; JOYPAD INTERRUPT  $0060
        RETI
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        

;===============================================================================
; Arithmetic ops
;===============================================================================

; 8 bit multiplication
; HL = H * E
;-------------------------------------------------------------------------------
___mul_8_::
        LD      D,  0
        LD      L,  D
        LD      B,  8
.loop:
        ADD     HL, HL
        JR      NC, .skip_add
        ADD     HL, DE
.skip_add:
        DEC     B
        JR      NZ, .loop
        RET

; 8 bit unsigned division and modulo
; D = D/E, A = D%E
;------------------------------------------------------------------------------- 
___div_u8_::
        XOR     A
        LD      B, 8
.loop:        
        SLA     D
        RLA
        CP      E
        JR      C, .skip_sub
        SUB     E
        INC     D
.skip_sub:
        DEC     B
        JR      NZ, .loop
        RET
        
; 152 bytes of free space $0068 - $00FF
;       DS      152
        DS      100

;===============================================================================
; Header
;===============================================================================
SECTION "___header___", ROM0[$0100]
; Startup code
        NOP
        JP      ___main

; Nintendo logo
        DB      $CE, $ED, $66, $66, $CC, $0D, $00, $0B
        DB      $03, $73, $00, $83, $00, $0C, $00, $0D
        DB      $00, $08, $11, $1F, $88, $89, $00, $0E
        DB      $DC, $CC, $6E, $E6, $DD, $DD, $D9, $99
        DB      $BB, $BB, $67, $63, $6E, $0E, $EC, $CC
        DB      $DD, $DC, $99, $9F, $BB, $B9, $33, $3E
        
; Title
        DB      "           "
        
; Manufacturer code
        DB      $BA, $AD, $F0, $0D
        
; Gameboy Color compatibility
        DB      0
        
; New licensee code
        DB      "  "
        
; Super Gameboy compatibility
        DB      0
        
; Cartridge Type
        DB      0

; Rom size
        DB      0

; Ram size
        DB      0
        
; Destination code
        DB      1
        
; Old licensee code
        DB      $33
        
; Rom version
        DB      0
        
; Header checksum
        DB      $FF
        
; Global checksum
        DW      $FFFF

;===============================================================================
; MAIN
;=============================================================================== 
SECTION "___main___", ROM0[$0150]
___main:
        DI
        LD      SP, $FFFE

        CALL    init_nums

        CALL    main        

.loop:
        HALT
        NOP
        JR      .loop
        
; SECTION "___wram_start___", WRAM0[$C000]

SECTION "vars", WRAM0
cur_y:  DW

SECTION "nums", ROM0
init_nums::
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
ld      [hl+], a
ld      [hl], a

; Load tiles
ld      hl, num_tiles
ld      de, $8000       ; VRAM
ld      b,  16          ; 17 tiles
.loop_1:
ld      a,  [hl+]
ld      [de], a
inc     de
dec     c
jr      nz, .loop_1
ld      c,  17
dec     b
jr      nz,  .loop_1

; Clear background
ld      hl, $9800
ld      b,  $20
ld      c,  $20
ld      a,  16          ; Empty tile
.loop_2:
ld      [hl+], a
dec     c
jr      nz, .loop_2
ld      c, $20
dec     b
jr      nz, .loop_2

ld      a, $91
ldh     [$40], a
ret


print_a::
ld      c, a
ld      b, 0
jr      print_hl.wait_vblank
print_hl::
ld      b, h
ld      c, l
.wait_vblank
ldh     a, [$44]    ; LCD_Y
cp      $90
jr      nz, .wait_vblank

; Value of cur_y in DE
ld      hl, cur_y
ld      a, [hl+]
ld      e, a
ld      a, [hl]
ld      d, a

ld      hl, $9800
add     hl, de

; print b
ld      a, b
swap    a
and     $0F
ld      [hl+], a
ld      a, b
and     $0F
ld      [hl+], a
; print c
ld      a, c
swap    a
and     $0F
ld      [hl+], a
ld      a, c
and     $0F
ld      [hl], a

ld      a, e
add     a, $20
ld      e, a
jr      nc, .skip
inc     d
.skip
ld      hl, cur_y
ld      [hl], e
inc     hl
ld      [hl], d
ret

num_tiles:
DB $00,$00,$3C,$3C,$66,$66,$6E,$6E
DB $7E,$7E,$76,$76,$66,$66,$3C,$3C
DB $00,$00,$18,$18,$78,$78,$18,$18
DB $18,$18,$18,$18,$18,$18,$7E,$7E
DB $00,$00,$3C,$3C,$66,$66,$06,$06
DB $1C,$1C,$30,$30,$66,$66,$7E,$7E
DB $00,$00,$3C,$3C,$66,$66,$06,$06
DB $1C,$1C,$06,$06,$66,$66,$3C,$3C
DB $00,$00,$0E,$0E,$1E,$1E,$36,$36
DB $66,$66,$7F,$7F,$06,$06,$06,$06
DB $00,$00,$7E,$7E,$60,$60,$7C,$7C
DB $06,$06,$06,$06,$66,$66,$3C,$3C
DB $00,$00,$1C,$1C,$30,$30,$60,$60
DB $7C,$7C,$66,$66,$66,$66,$3C,$3C
DB $00,$00,$7E,$7E,$66,$66,$06,$06
DB $0C,$0C,$18,$18,$30,$30,$30,$30
DB $00,$00,$3C,$3C,$66,$66,$66,$66
DB $3C,$3C,$66,$66,$66,$66,$3C,$3C
DB $00,$00,$3C,$3C,$66,$66,$66,$66
DB $3E,$3E,$06,$06,$0C,$0C,$38,$38
DB $00,$00,$18,$18,$3C,$3C,$66,$66
DB $66,$66,$7E,$7E,$66,$66,$66,$66
DB $00,$00,$7E,$7E,$33,$33,$33,$33
DB $3E,$3E,$33,$33,$33,$33,$7E,$7E
DB $00,$00,$1E,$1E,$33,$33,$60,$60
DB $60,$60,$60,$60,$33,$33,$1E,$1E
DB $00,$00,$7E,$7E,$36,$36,$33,$33
DB $33,$33,$33,$33,$36,$36,$7E,$7E
DB $00,$00,$7F,$7F,$31,$31,$34,$34
DB $3C,$3C,$34,$34,$31,$31,$7F,$7F
DB $00,$00,$7F,$7F,$31,$31,$34,$34
DB $3C,$3C,$34,$34,$30,$30,$78,$78
DB $00,$00,$00,$00,$00,$00,$00,$00
DB $00,$00,$00,$00,$00,$00,$00,$00






