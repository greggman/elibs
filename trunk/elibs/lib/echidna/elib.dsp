# Microsoft Developer Studio Project File - Name="elib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=elib - Win32 MultiThread DLL Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "elib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "elib.mak" CFG="elib - Win32 MultiThread DLL Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "elib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "elib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "elib - Win32 MFC Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "elib - Win32 MFC Release" (based on "Win32 (x86) Static Library")
!MESSAGE "elib - Win32 MultiThread DLL Release" (based on "Win32 (x86) Static Library")
!MESSAGE "elib - Win32 MultiThread DLL Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "elib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\elib.lib"

!ELSEIF  "$(CFG)" == "elib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\elibd.lib"

!ELSEIF  "$(CFG)" == "elib - Win32 MFC Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "elib___Win32_MFC_Debug"
# PROP BASE Intermediate_Dir "elib___Win32_MFC_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "elib___Win32_MFC_Debug"
# PROP Intermediate_Dir "elib___Win32_MFC_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\elib.lib"
# ADD LIB32 /nologo /out:"..\mfcelibd.lib"

!ELSEIF  "$(CFG)" == "elib - Win32 MFC Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "elib___Win32_MFC_Release"
# PROP BASE Intermediate_Dir "elib___Win32_MFC_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "elib___Win32_MFC_Release"
# PROP Intermediate_Dir "elib___Win32_MFC_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Z7 /O2 /D "WIN32" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\mfcelib.lib"
# ADD LIB32 /nologo /out:"..\mfcelib.lib"

!ELSEIF  "$(CFG)" == "elib - Win32 MultiThread DLL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "elib___Win32_MultiThread_DLL_Release"
# PROP BASE Intermediate_Dir "elib___Win32_MultiThread_DLL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "elib___Win32_MultiThread_DLL_Release"
# PROP Intermediate_Dir "elib___Win32_MultiThread_DLL_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\mtelib.lib"

!ELSEIF  "$(CFG)" == "elib - Win32 MultiThread DLL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "elib___Win32_MultiThread_DLL_Debug"
# PROP BASE Intermediate_Dir "elib___Win32_MultiThread_DLL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "elib___Win32_MultiThread_DLL_Debug"
# PROP Intermediate_Dir "elib___Win32_MultiThread_DLL_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D _EL_PLAT_WIN32__=1 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\elib.lib"
# ADD LIB32 /nologo /out:"..\mtelibd.lib"

!ENDIF 

# Begin Target

# Name "elib - Win32 Release"
# Name "elib - Win32 Debug"
# Name "elib - Win32 MFC Debug"
# Name "elib - Win32 MFC Release"
# Name "elib - Win32 MultiThread DLL Release"
# Name "elib - Win32 MultiThread DLL Debug"
# Begin Source File

SOURCE=.\argparse.c
# End Source File
# Begin Source File

SOURCE=.\checkglu.c
# End Source File
# Begin Source File

SOURCE=.\datafile.c
# End Source File
# Begin Source File

SOURCE=.\dbmess.c
# End Source File
# Begin Source File

SOURCE=.\eerrors.c
# End Source File
# Begin Source File

SOURCE=.\eio.c
# End Source File
# Begin Source File

SOURCE=.\ensure.c
# End Source File
# Begin Source File

SOURCE=.\exit.c
# End Source File
# Begin Source File

SOURCE=.\ezparse.c
# End Source File
# Begin Source File

SOURCE=.\hash.c
# End Source File
# Begin Source File

SOURCE=.\listapi.c
# End Source File
# Begin Source File

SOURCE=.\memfile.c
# End Source File
# Begin Source File

SOURCE=.\memsafe.c
# End Source File
# Begin Source File

SOURCE=.\photoshp.c
# End Source File
# Begin Source File

SOURCE=.\readgff.c
# End Source File
# Begin Source File

SOURCE=.\readgfx.c
# End Source File
# Begin Source File

SOURCE=.\readini.c
# End Source File
# Begin Source File

SOURCE=.\readpcx.c
# End Source File
# Begin Source File

SOURCE=.\readtga.c
# End Source File
# Begin Source File

SOURCE=.\strings.c
# End Source File
# Begin Source File

SOURCE=.\strings2.cpp
# End Source File
# Begin Source File

SOURCE=.\tmemsafe.c
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# End Target
# End Project
