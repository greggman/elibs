# Microsoft Developer Studio Generated NMAKE File, Based on eggsfilt.dsp
!IF "$(CFG)" == ""
CFG=eggsfilt - Win32 Debug
!MESSAGE No configuration specified. Defaulting to eggsfilt - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "eggsfilt - Win32 Release" && "$(CFG)" !=\
 "eggsfilt - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "eggsfilt.mak" CFG="eggsfilt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "eggsfilt - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "eggsfilt - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "eggsfilt - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\eggsfilt.exe"

!ELSE 

ALL : "$(OUTDIR)\eggsfilt.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\EGGSFILT.OBJ"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\eggsfilt.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)\eggsfilt.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\eggsfilt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\eggsfilt.pdb" /machine:I386 /out:"$(OUTDIR)\eggsfilt.exe" 
LINK32_OBJS= \
	"$(INTDIR)\EGGSFILT.OBJ"

"$(OUTDIR)\eggsfilt.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "eggsfilt - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\eggsfilt.exe"

!ELSE 

ALL : "$(OUTDIR)\eggsfilt.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\EGGSFILT.OBJ"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\eggsfilt.exe"
	-@erase "$(OUTDIR)\eggsfilt.ilk"
	-@erase "$(OUTDIR)\eggsfilt.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /D "_MBCS" /D _EL_PLAT_WIN32__=1 /Fp"$(INTDIR)\eggsfilt.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\eggsfilt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib elib.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\eggsfilt.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\eggsfilt.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\EGGSFILT.OBJ"

"$(OUTDIR)\eggsfilt.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "eggsfilt - Win32 Release" || "$(CFG)" ==\
 "eggsfilt - Win32 Debug"
SOURCE=.\EGGSFILT.C

!IF  "$(CFG)" == "eggsfilt - Win32 Release"

DEP_CPP_EGGSF=\
	".\switches.h"\
	{$(INCLUDE)}"echidna\argparse.h"\
	{$(INCLUDE)}"echidna\eerrors.h"\
	{$(INCLUDE)}"echidna\eio.h"\
	{$(INCLUDE)}"echidna\ensure.h"\
	{$(INCLUDE)}"echidna\listapi.h"\
	{$(INCLUDE)}"echidna\platform.h"\
	{$(INCLUDE)}"echidna\strings.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\EGGSFILT.OBJ" : $(SOURCE) $(DEP_CPP_EGGSF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "eggsfilt - Win32 Debug"

DEP_CPP_EGGSF=\
	".\switches.h"\
	{$(INCLUDE)}"echidna\argparse.h"\
	{$(INCLUDE)}"echidna\eerrors.h"\
	{$(INCLUDE)}"echidna\eio.h"\
	{$(INCLUDE)}"echidna\ensure.h"\
	{$(INCLUDE)}"echidna\listapi.h"\
	{$(INCLUDE)}"echidna\platform.h"\
	{$(INCLUDE)}"echidna\strings.h"\
	

"$(INTDIR)\EGGSFILT.OBJ" : $(SOURCE) $(DEP_CPP_EGGSF) "$(INTDIR)"


!ENDIF 


!ENDIF 

