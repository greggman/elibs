[start]
byte=1 2 3 4
#define goober=23
word=$1%goober%2
long=$ABCDDCBA
file="testdata.bin"
long=$FEDCBA98

#include "test2.lsp"

; file=			; pointer to a file
; data=			; pointer to a file (same as file=)

; pntr=			; pointer to another section
; level=		; pointer to another section (same as level=)

; long=			; longs (4 bytes each)
; word=			; words (2 bytes each)
; byte=			; bytes (1 byte each)
; float=		; floats (4 bytes each)
; string=		; null terminated strings
; binc=                 ; binary file to include here
; pad=			; boundry to pad to (no arg or 0 = default)

; path=			; set the path for loading binary files


