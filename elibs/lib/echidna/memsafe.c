/*************************************************************************
 *                                                                       *
 *                               MEMSAFE.C                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
		Memory management routines with heavy duty debugging features.

   PROGRAMMERS
		John M. Alvarado, Gregg Tavares

   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 GAT: Created

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "echidna/ensure.h"
#include "echidna/dbmess.h"
#include "echidna/memsafe.h"

/*************************** C O N S T A N T S ***************************/

#define MEM_INFOS_MAX         512
#define MEMSIG_LEN            16U                  // strlen(memsig)
#define TRACK_MEM_BUFFER_LEN  (MEMSIG_LEN * 2)
#if _EL_OS_MSDOS__
   #define GARBAGE_DATA          0xCC
#elif _EL_OS_IRIX53__
   #define GARBAGE_DATA          0xBD
#elif _EL_OS_WIN32__
   #define GARBAGE_DATA          0xCC
#elif _EL_OS_PSXOS__
   #define GARBAGE_DATA          0xED
#else
   #error GARBAGE_DATA constant not defined for this platform.
#endif


/******************************* T Y P E S *******************************/

typedef struct {
	void	*pvMemHeader;  // Debuging header and actual beginning of block.
	void	*pvMemUser;    // Beginning of users portion of block (after mem header).
	void	*pvMemFooter;  // Debugging footer of memory block.
	BOOL	 fUsed;        // Is block in use?
	int	 AllocIndex;   // Count index of when memory was allocated.
	UINT32 u32UserSize;  // User requested block size (not including mem header/footer).
   char  *pszMemName;   // User name of memory block.
   char  *pszFuncName;  // Name of function where memory was allocated.
   char  *pszFileName;  // Name of file where memory was allocated.
   int    LineNum;      // Linenumber in file at which memory was allocated.
   BOOL   fReferenced;  // Ever referenced?
} MEMINFO; // Naming: mi, *pmi

/************************** P R O T O T Y P E S **************************/

#if EL_DEBUG_MEMORY
   static BOOL fNewMemInfo (MEMINFO **ppmi);
   static void FreeMemInfo (MEMINFO *pmi);
   static BOOL fFindMemInfo (void *pv, MEMINFO **ppmi);
   static void CheckMemSigs (
      MEMINFO *pmi,
      char *pszFuncName,
      char *pszFileName,
      int LineNum
   );
   static void CheckMemName (
      MEMINFO *pmi,
      char *pszFuncName,
      char *pszFileName,
      int LineNum,
      char *pszMemName
   );
#endif

/***************************** G L O B A L S *****************************/

#if EL_DEBUG_MEMORY
   static MEMINFO armi[MEM_INFOS_MAX];
   static const char szMemSig[MEMSIG_LEN + 1] = "0123456789ABCDEF";
#endif

/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

#if EL_DEBUG_MEMORY
/***************************** EL_DEBUG_MEMORY ******************************/

/*************************************************************************
                             fAllocMem_Debug
 *************************************************************************

   SYNOPSIS
		BOOL fAllocMem_Debug (
		   		void **ppv,
		   		UINT32 u32Size,
		         MEMSAFETYPE mst,
		   		char *pszMemName,
		   		char *pszFileName,
		   		int LineNum
		   	   )

   PURPOSE
      To allocate the requested memory and set up a meminfo struct to 
      track it for debugging purposes.  

   INPUT
		ppv :          Non-Null ptr to mem ptr to set to the address of the allocated memory.
		u32UserSize :      Size of memory requested.
		mst :          Memory Type to allocate.
		pszMemName :   Name to use for allocated memory.
		pszFileName :  Name of of callers source file.
		LineNum :      Line number of call statement in callers source file.

   OUTPUT
		*ppv is set to address of allocated memory. *ppv set to NULL on failure.

   EFFECTS
		None

   RETURNS
      TRUE if able to allocate memory. FLASE if unable to allocate memory.

   SEE ALSO


   HISTORY
		08/11/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL fAllocMem_Debug (
   void **ppv,
   UINT32 u32UserSize,
   MEMSAFETYPE mst,
   char *pszMemName,
   char *pszFileName,
   int LineNum
)
BEGINFUNC (fAllocMem_Debug)
{
   static int NumAllocMemCalls = 0;
   MEMINFO *pmi;
   void *pvMem;
                         
   ENSURE_PTR(ppv);
   ENSURE(u32UserSize > 0);
   MEM_CheckMemStomp();
   
   *ppv = NULL;
   if (fAllocMem_Private (&pvMem, (u32UserSize + TRACK_MEM_BUFFER_LEN), mst))
   {
      if (fNewMemInfo(&pmi))
      {
         pmi->AllocIndex   = NumAllocMemCalls++;
         pmi->u32UserSize  = u32UserSize;

         pmi->pvMemHeader  = pvMem;
         pmi->pvMemUser    = (void *)((UINT8 HUGE32 *)pvMem + MEMSIG_LEN);
         pmi->pvMemFooter  = (void *)((UINT8 HUGE32 *)(pmi->pvMemUser) + u32UserSize);

         /*
         ** Write signitures before and after user memory to help detect 
         ** user writes that overflow or underflow the users buffer.  Also 
         ** if mst_Fill type not specified then shred users memory area 
         ** to help detect usage of uninitialized memory.
         */
         memcpy (pmi->pvMemHeader, szMemSig, MEMSIG_LEN);   // put signiture in header
         if (!(mst & mst_Fill))
         {
            memset (pmi->pvMemUser, GARBAGE_DATA, u32UserSize);
         }
         memcpy (pmi->pvMemFooter,  szMemSig, MEMSIG_LEN);  // put signiture in footer

         pmi->pszMemName   = pszMemName;
         pmi->pszFuncName  = pszFuncNameCaller();
         pmi->pszFileName  = pszFileName;
         pmi->LineNum      = LineNum;

         InfoMess(("%03d: %s (%s %d) allocates %s (0x%p [%lu]).\n",
               pmi->AllocIndex, pszFuncNameCaller(), pszFileName, LineNum,
                pszMemName, pmi->pvMemUser, (unsigned long)u32UserSize));

      	*ppv = pmi->pvMemUser;
      }
      else
      {
         FreeMem_Private(pvMem);
      }
   }

   RETURN (*ppv != NULL);

} ENDFUNC (fAllocMem_Debug)

/*************************************************************************
                          AllocMemNoFail_Debug
 *************************************************************************

   SYNOPSIS
		void AllocMemNoFail_Debug
		(
		   void **ppv,
		   UINT32 u32UserSize,
		   MEMSAFETYPE mst,
		   char *pszMemName,
		   char *pszFileName,
		   int LineNum
		)

   PURPOSE
		Same as AllocMem_Debug except it exits on failing

   INPUT
		ppv         :
		u32UserSize :
		mst         :
		pszMemName  :
		pszFileName :
		LineNum     :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		12/06/95 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void AllocMemNoFail_Debug
(
   void **ppv,
   UINT32 u32UserSize,
   MEMSAFETYPE mst,
   char *pszMemName,
   char *pszFileName,
   int LineNum
)
BEGINPROC (AllocMemNoFail_Debug)
{
	if (!fAllocMem_Debug (ppv, u32UserSize, mst, pszMemName, pszFileName, LineNum))
	{
		FailureMess (("%s (%s %d) Out of Memory: Couldn't allocate %ld bytes for %s in file %s\n",
         pszFuncNameCaller(), pszFileName, LineNum,
		 u32UserSize, pszMemName));
	}

} ENDPROC (AllocMemNoFail_Debug)


/*************************************************************************
                              FreeMem_Debug
 *************************************************************************

   SYNOPSIS
		void FreeMem_Debug (
		      void *pv,
		      char *pszMemName,
		      char *pszFileName,
		      int LineNum
		)

   PURPOSE
      To free memory allocated with fAllocMem_Debug

   INPUT
		pv          : Ptr to memory to be freed.
		pszMemName  : Name of the allocated memory.                      
		pszFileName : Name of the file of the calling function.
		LineNum     : Line number in the file where called from.

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO
      fAllocMem_Debug, fAllocMem, fFreeMem

   HISTORY
		08/15/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void FreeMem_Debug (
   void *pv,
   char *pszMemName,
   char *pszFileName,
   int LineNum
)
BEGINPROC (FreeMem_Debug)
{
   void *pvMemHeader;
   MEMINFO *pmi;

   ENSURE_PTR (pv);
   MEM_CheckMemStomp();


   if (fFindMemInfo(pv, &pmi))
   {
      InfoMess(("%s (%s %d) freeing %s (0x%p [%lu]).\n",
         pszFuncNameCaller(), pszFileName, LineNum, pszMemName, pv, (unsigned long)pmi->u32UserSize ));
      CheckMemName(pmi, pszFuncNameCaller(), pszFileName, LineNum, pszMemName);
      CheckMemSigs(pmi, pszFuncNameCaller(), pszFileName, LineNum);
      if (pv != pmi->pvMemUser)
      {
         FailureMess(("%s (%s %d) attempted to free 0x%p [%lu] subsection of larger allocation %s (0x%p [%lu]) allocated by %s (%s %d)",
            pszFuncNameCaller(), pszFileName, LineNum,
            pv, (unsigned long)pmi->u32UserSize,
            pmi->pszMemName, pmi->pvMemUser, (unsigned long)pmi->u32UserSize, pmi->pszFuncName, pmi->pszFileName, pmi->LineNum
         ));  
      }
      else
      {
         /*
         ** Scramble the memory with garbage before freeing it.
         */
         memset(pmi->pvMemHeader, GARBAGE_DATA, pmi->u32UserSize + TRACK_MEM_BUFFER_LEN);
         FreeMem_Private(pmi->pvMemHeader);
         FreeMemInfo (pmi);
      }
   }
   else
   {
      FailureMess (("%s (%s %d) attempted to free unowned memory %s (%p [?]).",
         pszFuncNameCaller(), pszFileName, LineNum,
         pszMemName, pv));
   }

} ENDPROC (FreeMem_Debug)

/*************************************************************************
                              fFindMemInfo
 *************************************************************************

   SYNOPSIS
		static BOOL fFindMemInfo (void *pv, MEMINFO **ppmi)

   PURPOSE
      To find the meminfo struct for the memory pointed at by pv.
      pv can point anywhere in the user memory of an allocation.

   INPUT
		pv    : Pointer to any portion of allocated user memory.
		ppmi  : Pointer to pointer to meminfo to return value in.

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      TRUE if found meminfo for memory. FALSE if none found.

   SEE ALSO


   HISTORY
		08/18/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static BOOL fFindMemInfo (void *pv, MEMINFO **ppmi)
BEGINFUNC (fFindMemInfo)
{
   int i;
   MEMINFO *pmi;

   *ppmi = NULL;
   for (i = 0, pmi=armi; i < MEM_INFOS_MAX; i++, pmi++)
   {
      if (pmi->fUsed && (pmi->pvMemUser <= pv && pv < pmi->pvMemFooter))
      {
         *ppmi = pmi;
         break;  
      }
   }

   RETURN (i < MEM_INFOS_MAX);

} ENDFUNC (fFindMemInfo)


/*************************************************************************
                              CheckMemName
 *************************************************************************

   SYNOPSIS
		static void CheckMemName (
         MEMINFO *pmi,
         char *pszFuncName,
         char *pszFileName,
         int LineNum,
         char *pszMemName
      )

   PURPOSE
      To see if the name of the memory being freed is the same as the name of the memory
      when it was allocated.

   INPUT
		pmi         : Pointer to MEMINFO struct for the memory.
      pszFuncName : Name of function to report error on.
		pszFileName : Name of source file of function.
		LineNum     : Line number in file
		pszMemName  : Name of memory user thinks he's referencing.

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/18/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void CheckMemName (
   MEMINFO *pmi,
   char *pszFuncName,
   char *pszFileName,
   int LineNum,
   char *pszMemName
)
BEGINPROC (CheckMemName)
{
   if (strcmp(pmi->pszMemName, pszMemName))
   {
      FailureMess (("%s (%s %d) memory name of \"%s\" does not match memory (0x%p [%lu]) allocated by %s (%s %d) as \"%s\".",
         pszFuncName, pszFileName, LineNum,
         pszMemName, pmi->pvMemUser, (unsigned long)pmi->u32UserSize,
         pmi->pszFuncName, pmi->pszFileName, pmi->LineNum, pmi->pszMemName));  
   }
} ENDPROC (CheckMemName)

/*************************************************************************
                              CheckMemSigs
 *************************************************************************

   SYNOPSIS
		static void CheckMemSigs (
         MEMINFO *pmi,
         char *pszFuncName,
         char *pszFileName,
         int LineNum)

   PURPOSE


   INPUT
		pmi         : Pointer to MEMINFO struct for the memory.
      pszFuncName : Name of function to report error on.
		pszFileName : Name of source file of function.
		LineNum     : Line number in file

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/18/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void CheckMemSigs (MEMINFO *pmi, char *pszFuncName, char *pszFileName, int LineNum)
BEGINPROC (CheckMemSigs)
{

   if (memcmp((char *)(pmi->pvMemHeader), szMemSig, MEMSIG_LEN))
   {
      FailureMess (("Underwrite of memory %s (0x%p [%lu]), allocated by %s (%s %d), detected at %s (%s %d).",
         pmi->pszMemName, pmi->pvMemUser, (unsigned long)pmi->u32UserSize,
         pmi->pszFuncName, pmi->pszFileName, pmi->LineNum,   
         pszFuncName, pszFileName, LineNum ));
   }
   if (memcmp((char *)(pmi->pvMemFooter), szMemSig, MEMSIG_LEN))
   {
      FailureMess (("Overwrite of memory  %s (0x%p [%lu]), allocated by %s (%s %d), detected at %s (%s %d).",
         pmi->pszMemName, pmi->pvMemUser, (unsigned long)pmi->u32UserSize,
         pmi->pszFuncName, pmi->pszFileName, pmi->LineNum,   
         pszFuncName, pszFileName, LineNum ));

   }
} ENDPROC (CheckMemSigs)

/*************************************************************************
                          ValidatePointer_Debug
 *************************************************************************

   SYNOPSIS
		void ValidatePointer_Debug (
		   void  *pv,
		   UINT32 u32Size,
		   char  *pszMemName,
		   char  *pszFileName,
		   int   LineNum
		)

   PURPOSE
      To determine if the given pointer points to a valid block of allocated
      memory.

   INPUT
		pv          : Pointer to validate.
		u32Size     : Amount of memory you think it is pointing at.
		pszMemName  : Name of memory you think your pointing at.
		pszFileName : Name of file of calling function.
		LineNum     : Line number in file of calling function.

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/15/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ValidatePointer_Debug (
   void  *pv,
   UINT32 u32Size,
   char  *pszMemName,
   char  *pszFileName,
   int   LineNum
)
BEGINPROC (ValidatePointer_Debug)
{
   MEMINFO *pmi;

   ENSURE_PTR(pv);
   ENSURE(u32Size != 0);

   if (fFindMemInfo(pv, &pmi))
   {
      CheckMemSigs(pmi, pszFuncNameCaller(), pszFileName, LineNum);
      if ( ((UINT8 HUGE32 *)pv + u32Size) > (UINT8 *)pmi->pvMemFooter)
      {
         FailureMess (("%s (%s %d) claim of %s (0x%p [%lu]) overflows %s (0x%p [%lu]), allocated by %s (%s %d), by %lu bytes.",
              pszFuncNameCaller(), pszFileName, LineNum,
              pszMemName, pv, (unsigned long)u32Size,
              pmi->pszMemName, pmi->pvMemUser, pmi->u32UserSize,
              pmi->pszFuncName, pmi->pszFileName, pmi->LineNum,
              (((UINT32)pv + u32Size) - (UINT32)(pmi->pvMemFooter))
              ));
      }
      pmi->fReferenced = TRUE;
   }
   else
   {
      FailureMess (("%s (%s %d) references unowned memory with %s (0x%p [?]).",
            pszFuncNameCaller(), pszFileName, LineNum,
            pszMemName, pv
         ));
   }

} ENDPROC (ValidatePointer_Debug)

/*************************************************************************
                           CheckMemStomp_Debug
 *************************************************************************

   SYNOPSIS
		void CheckMemStomp_Debug (
		   char  *pszFileName,
		   int   LineNum
		)

   PURPOSE
      Check if program has written outside the boundries of its memory
      allocations.

   INPUT
		pszFileName :
		LineNum :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/15/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CheckMemStomp_Debug (
   char  *pszFileName,
   int   LineNum
)
BEGINPROC (CheckMemStomp_Debug)
   TraceMessOff();
{
   int i;
   MEMINFO *pmi;

   for (i = 0, pmi=armi; i < MEM_INFOS_MAX; i++, pmi++)
   {
      if (pmi->fUsed)
      {
         CheckMemSigs(pmi, pszFuncNameCaller(), pszFileName, LineNum);
         if (!fMemOK_Private (pmi->pvMemHeader))
         {
            FailureMess (("Low level integrity check failure of memory %s (0x%p [%lu]), allocated by %s (%s %d), detected at %s (%s %d).",
               pmi->pszMemName, pmi->pvMemUser, (unsigned long)pmi->u32UserSize,
               pmi->pszFuncName, pmi->pszFileName, pmi->LineNum,   
               pszFuncNameCaller(), pszFileName, LineNum ));
         }
      }
   }

}
   TraceMessOn();
ENDPROC (CheckMemStomp_Debug)


/*************************************************************************
                           CheckMemLeak_Debug
 *************************************************************************

   SYNOPSIS
		void CheckMemLeak_Debug (
		 	VOIDFUNCTYPE NoteAllMemRefsFunc,
		   char  *pszFileName,
		   int   LineNum
		)

   PURPOSE
      To determine if any memory is no longer claimed by program but was
      never freed.

   INPUT
		NoteAllMemRefsFunc :
		pszFileName :
		LineNum :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/15/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CheckMemLeak_Debug (
 	VOIDFUNCTYPE NoteAllMemRefsFunc,
   char  *pszFileName,
   int   LineNum
)
BEGINPROC (CheckMemLeak_Debug)
   TraceMessOff();
{
   int i;
   MEMINFO *pmi;
   BOOL fFirst;

   MEM_CheckMemStomp();

   for (i = 0, pmi=armi; i < MEM_INFOS_MAX; i++, pmi++)
   {
      pmi->fReferenced = FALSE;
   }

   NoteAllMemRefsFunc();

   fFirst = TRUE;
   for (i = 0, pmi=armi; i < MEM_INFOS_MAX; i++, pmi++)
   {
      if (pmi->fUsed && !pmi->fReferenced)
      {
         if (fFirst)
         {
            FailureMess(("Memory leak detected at %s (%s %d):",
               pszFuncNameCaller(), pszFileName, LineNum));
            InfoMessAppend (("       Unreferenced allocation(s):\n"));
            fFirst = FALSE;
         }
         InfoMessAppend(("              %s (0x%p [%lu]) allocated by %s (%s %d)\n",
            pmi->pszMemName, pmi->pvMemUser, (unsigned long)pmi->u32UserSize,
            pmi->pszFuncName, pmi->pszFileName, pmi->LineNum));
      }
   }

}
   TraceMessOn();
ENDPROC (CheckMemLeak_Debug)

/*************************************************************************
                               fNewMemInfo
 *************************************************************************

   SYNOPSIS
		static BOOL fNewMemInfo (MEMINFO **ppmi)

   PURPOSE
      To get a free MEMINFO tracking struct.

   INPUT
		ppmi : Pointer to pointer to meminfo to set to free meminfo struct.

   OUTPUT
      *ppmi get pointer to free meminfo struct.

   EFFECTS
		None

   RETURNS
      TRUE. Halts program if unable to get free meminfo struct.

   SEE ALSO
      FreeMemInfo

   HISTORY
		08/16/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static BOOL fNewMemInfo (MEMINFO **ppmi)
BEGINFUNC (fNewMemInfo)
{
   int i;
   MEMINFO *pmi;

   for (i = 0, pmi=armi; i < MEM_INFOS_MAX; i++, pmi++)
   {
      if (!pmi->fUsed)
      {
         *ppmi = pmi;
         pmi->fUsed = TRUE;
         break;  
      }
   }
   ENSURE (i < MEM_INFOS_MAX);

   RETURN (TRUE);

} ENDFUNC (fNewMemInfo)

/*************************************************************************
                               FreeMemInfo
 *************************************************************************

   SYNOPSIS
		static void FreeMemInfo (MEMINFO *pmi)

   PURPOSE
      To put meminfo struct back in free pool.

   INPUT
		pmi : Pointer to meminfo to free up.

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO
      fNewMemInfo

   HISTORY
		08/16/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void FreeMemInfo (MEMINFO *pmi)
BEGINPROC (FreeMemInfo)
{
   ENSURE_PTR (pmi);
   ENSURE ((pmi >= armi) && (pmi < (armi+MEM_INFOS_MAX)));

   pmi->fUsed = FALSE;
   pmi->pvMemHeader = NULL;
   pmi->pvMemUser = NULL;
   pmi->pvMemFooter = NULL;

} ENDPROC (FreeMemInfo)

#endif



