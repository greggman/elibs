#
# Name of program.
#
PROGRAM   = tpmckid2

#
# Program Modules.
#
PDEP =	\
		$(PROGRAM).obf	\

ODEP =	\
		version.obj	\

BLA = \
		$(ELIB)\echidna\memlib\memdbug.obj	\

#
# Memory Model
#   s = Small
#   m = Medium
#   c = Compact
#   l = Large
#   h = Huge
MDL		= w

ELDEP =	\
		echidna\beiff$(MDL).lib	\
		echidna\lds$(MDL).lib	\
		echidna\mem$(MDL).lib	\
		echidna\misc$(MDL).lib	\

LDEP = \


#
# Name of linker file
#
LINKFILE  = linkfile.lnk

#
# 'C' compiler and global compiler flags.
#
#
# 'C' compiler and global compiler flags.
#
CFLAGS    = $(CMN_CFLAGS)
CCOMPILER = $(CMN_CC) 

#
# Assembler and global assembler flags.
#
ASSEMBLER = $(CMN_ASM)
AFLAGS    = $(CMN_AFLAGS)

#
# Linker and global linker flags + libraries to like with.
#
LINKER    = $(CMN_LINK)

#
# Rule for Assembly modules.
#
.asm.obj:
	$(ASSEMBLER) $(AFLAGS) $*.asm

#
# Rule for 'C' modules.
#
.c.obj:
	$(CCOMPILER) $(CFLAGS) $*.c

.c.obf:
	$(CCOMPILER) $(CFLAGS) $(CMN_CSPEED) /fo$*.obf $*.c

#
# Dependency for program.
#
$(PROGRAM).exe: $(PDEP)
	bump version.c
	$(CCOMPILER) $(CFLAGS) version.c
	makescan WPROGRAM MAKEFILE makefile.w
	$(LINKER) @$(LINKFILE)
	dir $(PROGRAM).EXE
#
# extra dependencies
#
