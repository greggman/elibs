[start,align=$400]
string=<this is the start>
align=4
%fif(%fexists(c:\temp\foo),string=***foo exists***,byte=$DE $AD $BE $EF)
%fif(%fexists(c:/temp/foo5),string=***foo5 exists***,byte=$DE $AD $BE $EF)
pntr=goobersnot
file=testdata.bin,align=128
string=abc
insert=other
string=jkl
insert=another
string=pqr
byte=1 2 3 4
insert=another
insert=another
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

[other]
string=def
string=ghi

[another]
string=mno


[goobersnot,align=256]
string=hello

