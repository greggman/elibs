/*************************************************************************
 *                                                                       *
 *                               MEMSAFE.H                               *
 *                                                                       *
 *************************************************************************

                           Copyright 1996 Echidna

   DESCRIPTION
		Memory management routines with heavy duty debugging features.

   PROGRAMMERS
		John M. Alvarado

   FUNCTIONS

   TABS : 4 7

   HISTORY
		08/08/95 JMA: Created.

 *************************************************************************/

#ifndef MEMSAFE_H
#define MEMSAFE_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#ifdef __cplusplus
extern "C" {
#endif
/*************************** C O N S T A N T S ***************************/

/*
** This value is used to subtract the debug code size from the memory hid 
** from the game to simulate consumer hardware memory constraints on a 
** development station.  It should be calculated by compiling the program 
** with and without the debugging code on and taking the difference in the 
** total memory the game can allocate at startup.
*/
#define ulSizeOfDebugCode  (1024 * 0)  // a temp value for now.

/******************************* T Y P E S *******************************/

/*
** Memory types.  These may not be combined to form complex types.  If 
** complex types are required, then the supporter of memsafe should add 
** them as new entries to the enumeration.  Example: mst_Audio_Temp 
** for temporary audio memory.  Any such new entries should order the 
** modifiers alphabetically within the new name (e.g.  mst_Audio_Bank1 
** and not mst_Bank1_Audio)
*/
typedef UINT32 MEMSAFETYPE;   // mst
#define mst_Any        (MEMSAFETYPE)0x00000000
#define mst_Temp       (MEMSAFETYPE)0x00008000
#define mst_Audio      (MEMSAFETYPE)0x00004000
#define mst_Video      (MEMSAFETYPE)0x00002000
#define mst_Fill       (MEMSAFETYPE)0x00001000 // memory set to value of low 8 bits.
typedef void (*VOIDFUNCTYPE)(void);

/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/

/* This is because the damned diab compiler preprocessor is broken */
#define ARGTOSTR(arg)    #arg

#if EL_DEBUG_MEMORY
   #if (EL_NO_ENSURE || !EL_DEBUG_MESSAGES)
      #error  You must turn off EL_NO_ENSURE and turn on EL_DEBUG_MESSAGES to get reports from EL_DEBUG_MEMORY routines!       
   #endif
   #define MEM_fOpenMemSys(u32SizeSpared, u32SizeHid)        fOpenMemSys_Private(u32SizeSpared,                 \
                                                            ( ((u32SizeHid) <= ulSizeOfDebugCode) ?         \
                                                               0 : ((u32SizeHid) - ulSizeOfDebugCode) ) )
   #define MEM_fAllocMemTypeNamed(pv,u32Size,mst,MemName)    fAllocMem_Debug ((void **)&(pv), u32Size, \
                                                                           (mst), \
                                                                           MemName, __FILE__, __LINE__)
   #define MEM_AllocMemTypeNamedNoFail(pv,u32Size,mst,MemName)    AllocMemNoFail_Debug ((void **)&(pv), u32Size, \
                                                                           (mst), \
                                                                           MemName, __FILE__, __LINE__)
   #define MEM_fAllocMemTypeNamedFileLine(pv,u32Size,mst,MemName,szFile, Line)    \
                                                         fAllocMem_Debug (&(pv), u32Size, mst, \
                                                                            MemName, szFile, Line)
   #define MEM_FreeMemNamed(pv, MemName)                     FreeMem_Debug (pv, MemName, __FILE__, __LINE__)
   #define MEM_ValidatePointerNamed(pv, u32Size, MemName)    ValidatePointer_Debug (pv, u32Size, MemName, __FILE__, __LINE__)
   #define MEM_CheckMemStomp()                               CheckMemStomp_Debug   (                       __FILE__, __LINE__)
   #define MEM_CheckMemLeak(func)                            CheckMemLeak_Debug    (func,                  __FILE__, __LINE__)
   #define MEM_PrintMemState()                               PrintMemState_Private ()

#else
   #define MEM_fOpenMemSys(u32SizeSpared, u32SizeHid)        fOpenMemSys_Private(u32SizeSpared, u32SizeHid)
   #define MEM_fAllocMemTypeNamed(pv,u32Size,mst,MemName)    fAllocMem_Private((void **)&(pv), u32Size, mst)
   #define MEM_AllocMemTypeNamedNoFail(pv,u32Size,mst,MemName)    AllocMemNoFail_Private ((void **)&(pv), u32Size, mst)
   #define MEM_fAllocMemTypeNamedFileLine(pv,u32Size,mst,MemName, szFile, Line) \
														MEM_fAllocMemTypeNamed(pv,u32Size,mst,MemName)
   #define MEM_FreeMemNamed(pv, MemName)                     FreeMem_Private(pv)
   #define MEM_ValidatePointerNamed(pv, u32Size, MemName)
   #define MEM_CheckMemStomp() 
   #define MEM_CheckMemLeak(func)
   #define MEM_PrintMemState()
#endif

/********************* Mem of Type and Named Interface *********************/
//      MEM_fAllocMemTypeNamed(pv, u32Size, mst, MemName)       defined above
#define MEM_fCallocMemTypeNamed(pv, u32Size, mst, MemName)      MEM_fAllocMemTypeNamed  (pv, u32Size, (mst_Fill|mst), MemName)
#define MEM_fAllocTempMemTypeNamed(pv, u32Size, mst, MemName)   MEM_fAllocMemTypeNamed  (pv, u32Size, (mst_Temp|mst), MemName)
#define MEM_fCallocTempMemTypeNamed(pv, u32Size, mst, MemName)  MEM_fCallocMemTypeNamed (pv, u32Size, (mst_Temp|mst), MemName)

/************************** Mem Named Interface **************************/
#define MEM_fAllocMemNamed(pv, u32Size, MemName)                MEM_fAllocMemTypeNamed      (pv, u32Size, mst_Any, MemName)
#define MEM_fCallocMemNamed(pv, u32Size, MemName)               MEM_fCallocMemTypeNamed     (pv, u32Size, mst_Any, MemName)
#define MEM_fAllocTempMemNamed(pv, u32Size, MemName)            MEM_fAllocTempMemTypeNamed  (pv, u32Size, mst_Any, MemName)
#define MEM_fCallocTempMemNamed(pv, u32Size, MemName)           MEM_fCallocTempMemTypeNamed (pv, u32Size, mst_Any, MemName)
//      MEM_FreeMemNamed                                        defined above.

//      MEM_ValidatePointerNamed(pv, u32Size, MemName)          defined above.
#define MEM_NoteMemRefNamed(pv,u32Size, MemName)                MEM_ValidatePointerNamed (pv, u32Size, MemName)

/************************** Mem of Type Interface **************************/
#define MEM_fAllocMemType(pv, u32Size, mst)                     MEM_fAllocMemTypeNamed      (pv, u32Size, mst, ARGTOSTR(pv))
#define MEM_fCallocMemType(pv, u32Size, mst)                    MEM_fCallocMemTypeNamed     (pv, u32Size, mst, ARGTOSTR(pv))
#define MEM_fAllocTempMemType(pv, u32Size, mst)                 MEM_fAllocTempMemTypeNamed  (pv, u32Size, mst, ARGTOSTR(pv))
#define MEM_fCallocTempMemType(pv, u32Size, mst)                MEM_fCallocTempMemTypeNamed (pv, u32Size, mst, ARGTOSTR(pv))

/**************************** Simple Interface  ****************************/
//      fOpenMemSys(u32SizeSpared, u32SizeHid)              defined above.
#define MEM_CloseMemSys()                                       CloseMemSys_Private()
#define MEM_fOpenMemScope()                                     fOpenMemScope_Private()
#define MEM_CloseMemScope()                                     CloseMemScope_Private()
                                                      
#define MEM_fAllocMem(pv, u32Size)                              MEM_fAllocMemType      (pv, u32Size, mst_Any)
#define MEM_fCallocMem(pv, u32Size)                             MEM_fCallocMemType     (pv, u32Size, mst_Any)
#define MEM_fAllocTempMem(pv, u32Size)                          MEM_fAllocTempMemType  (pv, u32Size, mst_Any)
#define MEM_fCallocTempMem(pv, u32Size)                         MEM_fCallocTempMemType (pv, u32Size, mst_Any)
#define MEM_FreeMem(pv)                                         MEM_FreeMemNamed (pv, ARGTOSTR(pv))

#define MEM_ValidatePointer(pv, u32Size)                        MEM_ValidatePointerNamed(pv, u32Size, ARGTOSTR(pv))
//      MEM_CheckMemStomp()                                     defined above.
//      MEM_CheckMemLeak(func)                                  defined above.
#define MEM_NoteMemRef(pv,u32Size)                              MEM_NoteMemRefNamed(pv, u32Size, ARGTOSTR(pv))
//      MEM_PrintMemState()                                     defined above.

/*************************** No fail interface ***************************/

#define MEM_CallocMemTypeNamedNoFail(pv, u32Size, mst, MemName) MEM_AllocMemTypeNamedNoFail  (pv, u32Size, (mst_Fill|mst), MemName)
#define MEM_AllocMemTypeNoFail(pv, u32Size, mst)                MEM_AllocMemTypeNamedNoFail (pv, u32Size, mst, ARGTOSTR(pv))
#define MEM_CallocMemTypeNoFail(pv, u32Size, mst)               MEM_CallocMemTypeNamedNoFail(pv, u32Size, mst, ARGTOSTR(pv))

#define MEM_AllocMemNoFail(pv, u32Size)                         MEM_AllocMemTypeNoFail      (pv, u32Size, mst_Any)
#define MEM_CallocMemNoFail(pv, u32Size)                        MEM_CallocMemTypeNoFail     (pv, u32Size, mst_Any)

#define MEM_AllocTempMemNoFail(pv,u32Size)						 MEM_AllocMemTypeNoFail      (pv, u32Size, mst_Temp)
#define MEM_CallocTempMemNoFail(pv,u32Size)					 MEM_AllocMemTypeNoFail      (pv, u32Size, (mst_Fill|mst_Temp))

/************************** P R O T O T Y P E S **************************/

#if EL_DEBUG_MEMORY

   BOOL fAllocMem_Debug (
      void **ppv,
      UINT32 u32Size,
      MEMSAFETYPE mst,
      char *pszMemName,
      char *pszFileName,
      int LineNum
      );
   void AllocMemNoFail_Debug (
      void **ppv,
      UINT32 u32Size,
      MEMSAFETYPE mst,
      char *pszMemName,
      char *pszFileName,
      int LineNum
      );
   void FreeMem_Debug (
   		void *pv,
   		char *pszMemName,
   		char *pszFileName,
   		int LineNum
   	   );
   void ValidatePointer_Debug (
   		void  *pv,
         UINT32 u32Size,
   		char  *pszMemName,
   		char  *pszFileName,
   		int   LineNum
         );
   void CheckMemStomp_Debug(
   		char  *pszFileName,
   		int   LineNum
         );
   void CheckMemLeak_Debug(
   		VOIDFUNCTYPE NoteAllMemRefsFunc,
   		char  *pszFileName,
   		int   LineNum
         );
   void PrintMemState_Private(void);
   BOOL fMemOK_Private (void *pv);
#endif

BOOL fOpenMemSys_Private(uint32 u32SizeSpared, uint32 u32SizeHid);
void CloseMemSys_Private(void);
BOOL fOpenMemScope_Private(void);
void CloseMemScope_Private(void);
BOOL fAllocMem_Private (void **ppv, UINT32 u32Size, MEMSAFETYPE mst);
void AllocMemNoFail_Private (void **ppv, UINT32 u32Size, MEMSAFETYPE mst);
void FreeMem_Private(void *pv);


#ifdef __cplusplus
}
#endif

#endif /* MEMSAFE_H */





