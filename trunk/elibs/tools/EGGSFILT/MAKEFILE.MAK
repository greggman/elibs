#
# Name of program.
#
PROGRAM   = echidnafilt

#
# Program Modules.
#
PDEP =	\
		$(PROGRAM).obj	\

ODEP =	\

#
# Memory Model
#   s = Small
#   m = Medium
#   c = Compact
#   l = Large
#   h = Huge
MDL		= h
STARTUP	= c0$(MDL).obj

#
# Libraries
#
ELDEP =		\
	echidna\lds$(MDL).lib	\
	echidna\misc$(MDL).lib	\

LDEP =		\
	c$(MDL).lib


#
# Name of linker file
#
LINKFILE  = linkfile.lnk

#
# 'C' compiler and global compiler flags.
#
CFLAGS    = $(CMN_CFLAGS) -m$(MDL)
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
LFLAGS    = /c /x /v /m
#LFLAGS    = /c /x
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
# Rule for 'Y' modules.
#
.y.c:
	$(YACC) $(YFLAGS) $*.y

#
# Dependency for program.
#
$(PROGRAM).exe: $(PDEP)
	makescan
	$(LINKER) $(LFLAGS) @$(LINKFILE), $(PROGRAM).EXE, , @$(LIBFILE)
	dir $(PROGRAM).EXE

#
# extra dependencies
#

