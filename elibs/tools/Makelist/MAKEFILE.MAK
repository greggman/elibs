#
# Name of program.
#
PROGRAM   = makelist

#
# Program Modules.
#
PDEP =	\
		$(PROGRAM).obj	\

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
MDL = x32
STARTUP	= c0$(MDL).obj

ELDEP =	\
		echidna\lds$(MDL).lib	\
		echidna\mem$(MDL).lib	\
		echidna\misc$(MDL).lib	\
		echidna\beiff$(MDL).lib	\

LDEP = \
		DPMI32	\
		CW32	\


#
# Name of linker file
#
LINKFILE  = linkfile.lnk

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
LFLAGS    = $(CMN_LFLAGS)
LIBFILE   = libs.lnk

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

#
# Dependency for program.
#
$(PROGRAM).exe: $(PDEP)
	bump version.c
	$(CCOMPILER) $(CFLAGS) version.c
	makescan
	$(LINKER) $(LFLAGS) @$(LINKFILE), $(PROGRAM).EXE, , @$(LIBFILE)
	dir $(PROGRAM).EXE
