/*************************************************************************
 *                                                                       *
 *                              GMEMSAFE.C                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
      Game Memory Management subsystem to memsafe.c

   PROGRAMMERS
		John M. Alvarado, Gregg Tavares

   FUNCTIONS

   TABS : 4 7

   HISTORY
		07/09/96 GAT: Created

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "echidna/memsafe.h"
#include "echidna/dbmess.h"


/*************************** C O N S T A N T S ***************************/

// This is how many largest pieces of memory OpenGMem will take from 
// system on first call (init call).
#define nGMemPiecesMax  2     // DRAM and VRAM

// This is how many times -2 you can call OpenGMem (without calling 
// CloseGMem) before overflowing gmem stack.
#define nGMemScopeMax   6 //4     

#define sizeofSmallConcession (1024)

/******************************* T Y P E S *******************************/

typedef struct
{
   MEMSAFETYPE mst;        // Type flags for memory.
   UINT8 *pu8MemStart;     // first byte in memory piece.
   UINT8 *pu8MemEnd;       // first byte not in memory piece (== pu8MemStart + sizeof_mempiece).
} MEMPIECE; // memp

typedef struct
{
   MEMPIECE armemp[nGMemPiecesMax];
} GMEMSCOPE; // gmemscope

typedef struct 
{
   INT32       cPieces;       // Number of pieces of memory tracked.
   INT32       igmemscope;    // Current GMem Scope. 0..nGMemScopeMax
                              // 0 = Uninitialized.1
                              // 1 = Initialized, first scope.
                              // nGMemScopeMax = at lowest nest level.
   // argmemscope[0] holds values of intial memory grab from system.
   // argmemscope[1] holds floating pointers that reflect allocs/frees within
   //    first scope.  This is initialized on first call to OpenGMem.
   // Each entry after [0] starts with current values in previous entry.
   // For example, the second call to OpenGMem sets initializes thus:
   //    argmemscope[2] = argmemscope[1];
   GMEMSCOPE   argmemscope[nGMemScopeMax+1];
} GMEMSYS;


/************************** P R O T O T Y P E S **************************/

static BOOL MemTypeMatch (MEMSAFETYPE mstReq, MEMSAFETYPE mstMem);
static MEMSAFETYPE mstOfSystemMemType (void *pv);

/***************************** G L O B A L S *****************************/

static GMEMSYS  gmemsys;

#if _EL_OS_PSXOS__
   INT32 arsizeofWayTooMuch[nGMemPiecesMax] = 
   {
      (1024 * 1024 * 8),
   };
	#if PSX_FINAL
		#define MEMMONSTER	(1024*(1085+161))
		//#define MEMMONSTER	(1024*1024 + 1024*1024)
	#else
	//   #define MEMMONSTER   (1024*1024*7 + 1024*(512)) //512)
		#define MEMMONSTER   (1024*1024*6) //7 + 1024*(512)) //512)
	#endif
   static char membuf[MEMMONSTER];
#endif

/****************************** M A C R O S ******************************/
#define SizeOfFreeGMemPiece(pmemp)    \
   (UINT32)(pmemp->pu8MemEnd - pmemp->pu8MemStart)

#if _EL_OS_PSXOS__
   #define GetPageSize(memtype)  1
   #define ScavengeMem()
#endif

/**************************** R O U T I N E S ****************************/

/*************************************************************************
                           fOpenMemSys_Private
 *************************************************************************

   SYNOPSIS
		BOOL fOpenMemSys_Private (UINT32 u32SizeSpared, UINT32 u32SizeHid)

   PURPOSE
      To initalize gmem system.

      This function should be accessed only through macros defined in 
      memsafe.h.  Do not call directly.

   INPUT
		u32SizeSpared: Amount of memory to leave to OS.
      u32SizeHid:    Amount of memory to hide from OS and from game, for simulating
                     amount of memory available in consumer machines.

   OUTPUT
		None

   EFFECTS
      Allocates the first nGMemPiecesMax biggest chunks of 
      memory from the system.

   RETURNS
      TRUE on success. FALSE on failure.

   SEE ALSO
      OpenMemSys macro in memsafe.h
      CloseMemSys_Private, CloseMemSys macro in memsafe.h

   HISTORY
		08/11/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL fOpenMemSys_Private (UINT32 u32SizeSpared, UINT32 u32SizeHid)
BEGINFUNC (OpenMemSys_Private)
{
   int i;               
   UINT32 sizeReqCrnt;  // Current size of memory request to system.
   MEMPIECE *pmemp;     // Pointer to currently allocated piece.
   UINT32 TotalMem;     // Total running count of allocated memory.
   MEMSAFETYPE mst;     // Type of memory of currently allocated piece.
   UINT8    *pu8Spared; // Memory that is freed back to system.
   #if 0 //
   UINT32 u32SysLargest;
   UINT32 u32TaskLargest;
   #endif
   INT32 PageSize;

   // Is this a second call to OpenMemSys_Private()? 
   if (0 != gmemsys.igmemscope)
   {
      WarningMess (("Multiple calls to fOpenMemSys not allowed."));
      RETURN FALSE;
   }

   /*
   ** Pad spared size to system memory page size
   */
   {
      INT32 PagesSpared;
      PageSize = GetPageSize(MEMTYPE_NORMAL);
      PagesSpared = (u32SizeSpared + (PageSize-1)) / PageSize;
      u32SizeSpared = PagesSpared * PageSize;
   }
   
   /* Pad hidden size to system word alignment size */
   u32SizeHid = (INT32)AlignedUpVal (u32SizeHid);
   gmemsys.cPieces = 0;
   TotalMem = 0;

   /*
   ** Set aside the memory that will be spared back to system
   */
   // First find largest piece
   #if 0
   {
      MemInfo meminfo;
      GetMemInfo (&meminfo, sizeof (meminfo), MEMTYPE_NORMAL);

	  u32SysLargest  = meminfo.minfo_LargestFreePageSpan;
	  u32TaskLargest = meminfo.minfo_TaskAllocatedPages * PageSize - meminfo.minfo_TaskAllocatedBytes;
   }
   #endif
   
   pu8Spared = NULL;
   if (u32SizeSpared)
   {
      #if 0
      ENSURE_F(u32SysLargest >= u32SizeSpared, ("Not enough contiguous memory in system to spare %lu bytes.", u32SizeSpared));
      #endif
      pu8Spared = (UINT8 *)malloc (u32SizeSpared);  
      ENSURE_(pu8Spared != NULL, "Could not allocate spared memory block.");
   }

   /* Allocate first nGMemPiecesMax largest pieces of mem from system */
   for (i = 0, pmemp = gmemsys.argmemscope[0].armemp; i < nGMemPiecesMax; i++, pmemp++)
   {
      UINT8 *pub;

      pub = NULL;

      #if _EL_OS_PSXOS__
         #if 0
         /*
         ** Get biggest available piece by asking first for too much, 
         ** then repeatedly asking for slightly smaller pieces until it 
         ** gets some.
         */
         sizeReqCrnt = arsizeofWayTooMuch[i];
         while (!pub && sizeReqCrnt)
         {
            pub = (uint8 *)malloc (sizeReqCrnt);  
            if (pub)
            {
               mst = mst_Any; //mstOfSystemMemType((void *)pub); // mstOfSystemMemType not written yet.
               break;
            }
            // Prepare to ask again for a slightly smaller piece.
            sizeReqCrnt -= sizeofSmallConcession;
         }
         #else
            if (i == 0)
            {
               pub = membuf;
               mst = mst_Any|mst_Fill|mst_Temp; //mstOfSystemMemType((void *)pub); // mstOfSystemMemType not written yet.
               sizeReqCrnt = MEMMONSTER - u32SizeSpared;
            }
         #endif
      #elif 0
         {
		   {
			  MemInfo meminfo;
			  GetMemInfo (&meminfo, sizeof (meminfo), MEMTYPE_NORMAL);
		
			  u32SysLargest  = meminfo.minfo_LargestFreePageSpan;
			  u32TaskLargest = meminfo.minfo_TaskAllocatedPages * PageSize - meminfo.minfo_TaskAllocatedBytes;

			  InfoMess (("minfo_LargestFreePageSpan = %ld\n", meminfo.minfo_LargestFreePageSpan));

			  InfoMess (("SysLargest = %ld, TaskLargest = %ld, PageSize = %ld\n",
  				u32SysLargest, u32TaskLargest, PageSize));
		   }

            sizeReqCrnt = u32SysLargest;
            if (sizeReqCrnt < u32TaskLargest)
            {
               sizeReqCrnt = u32TaskLargest;
            }
            if (sizeReqCrnt)
            {
               pub = (UINT8 *)malloc (sizeReqCrnt);  
            }
            mst = (mst_Temp|mst_Audio|mst_Video|mst_Fill);
         }
      #else
         #error fOpenMemSys_Private() code not defined for this platform.
      #endif

      if (pub)
      {
         pmemp->mst = mst;
         pmemp->pu8MemStart = pub; 
         pmemp->pu8MemEnd = pub + sizeReqCrnt; 
         ++gmemsys.cPieces;   // Count this piece.
         #if 0
         InfoMess (("MemPiece[%02ld]: (0x%08p [%7lu]) Type [%s%s%s%s]\n",
                     gmemsys.cPieces, pub, (unsigned long)sizeReqCrnt,
                     pszIf(mst & mst_Temp, "Temp "), 
                     pszIf(mst & mst_Audio, "Audio "),
                     pszIf(mst & mst_Video, "Video "),
                     pszIf(mst & mst_Fill, "Fill ")
            ));
         #endif            
         TotalMem += sizeReqCrnt;
      }

   }

   if (pu8Spared)
   {
      free (pu8Spared); // put in tasks free list.
      ScavengeMem ();   // let system transfer it to systems free list.
   }

   // Did I fail to get any memory from system?
   if (!gmemsys.cPieces)
   {
	  FailureMess (("You lose, no memory!"));
      RETURN FALSE;
   }

//   InfoMess (("Total Game Memory =        %7lu\n", (unsigned long)TotalMem));

   /* Push initial  scope */
   fOpenMemScope_Private();

   /* Hide memory */
   pmemp = gmemsys.argmemscope[gmemsys.igmemscope].armemp;
   if (SizeOfFreeGMemPiece(pmemp) < u32SizeHid)
   {
      InfoMess (("Not enough memory to hide %lu.", u32SizeHid));
      CloseMemSys_Private();
      RETURN FALSE;
   }

   pmemp->pu8MemEnd -= u32SizeHid;
//   InfoMess (("Spared Memory = 0x%08lx = %7lu\n", (unsigned long)u32SizeSpared, (unsigned long)u32SizeSpared));
//   InfoMess (("Hidden Memory = 0x%08lx = %7lu\n", (unsigned long)u32SizeHid, (unsigned long)u32SizeHid));


   RETURN TRUE;

} ENDFUNC (OpenMemSys_Private)


/*************************************************************************
                          fOpenMemScope_Private
 *************************************************************************

   SYNOPSIS
		BOOL fOpenMemScope_Private (void)

   PURPOSE
      To preserve the current state of memory allocation for subsequent restoration
      by CloseMemScope_Private.

   INPUT
		None

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      TRUE on success. FALSE on failure (out of scope tracking space).

   SEE ALSO
      OpenMemScope macro in memsafe.h
      CloseMemScope_Private, CloseMemScope macro in memsafe.h

   HISTORY
		08/24/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL fOpenMemScope_Private (void)
BEGINFUNC (fOpenMemScope_Private)
{
   ENSURE_(gmemsys.igmemscope < nGMemScopeMax, "Memory scope stack overflow.");
   /* Push new scope */
   ++gmemsys.igmemscope;
   gmemsys.argmemscope[gmemsys.igmemscope] = 
      gmemsys.argmemscope[gmemsys.igmemscope-1];

   RETURN TRUE;
   

} ENDFUNC (fOpenMemScope_Private)

/*************************************************************************
                          CloseMemScope_Private
 *************************************************************************

   SYNOPSIS
		void CloseMemScope_Private (void)

   PURPOSE
      To restore memory allocation state to that which held at the last
      call to fOpemMemScope_Private.

   INPUT
		None

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO
      fOpenMemScope_Private, CloseMemScope macro in memsafe.h

   HISTORY
		08/24/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CloseMemScope_Private (void)
BEGINPROC (CloseMemScope_Private)
{
   ENSURE_(gmemsys.igmemscope > 0, "Memory scope stack underflow.");
   /* Pop gmem scope */
      --gmemsys.igmemscope;

} ENDPROC (CloseMemScope_Private)

/*************************************************************************
                            CloseMemSys_Private
 *************************************************************************

   SYNOPSIS
		void CloseMemSys_Private (void)

   PURPOSE
      To restore memory to the system and undo whatever was done by
      call to OpenMemSys_Private.

      This function should be accessed only through macro CloseMemSys defined in 
      memsafe.h.  Do not call directly.

   INPUT
		None

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO
      OpenMem_Private, CloseMemSys macro in memsafe.h

   HISTORY
		08/11/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CloseMemSys_Private (void)
BEGINPROC (CloseMemSys_Private)
{
   int i;
   MEMPIECE *pmemp;

   if (gmemsys.igmemscope > 1)
   {
      WarningMess (("Memory system closing with open scopes outstanding."));
   }
   if (gmemsys.igmemscope < 1)
   {
      WarningMess (("Multiple calls to CloseMemSys not allowed."));
   }

   /*
   ** Shut down entire GMem system, freeing memory back to host OS
   */
   for (i = 0, pmemp = gmemsys.argmemscope[0].armemp; i < gmemsys.cPieces; i++, pmemp++)
   {
      free (pmemp->pu8MemStart);
   }
   gmemsys.igmemscope = 0;
   gmemsys.cPieces = 0;

} ENDPROC (CloseMemSys_Private)

/*************************************************************************
                            fAllocMem_Private
 *************************************************************************

   SYNOPSIS
		BOOL fAllocMem_Private (void **ppv, UINT32 u32Size, MEMSAFETYPE mst)

   PURPOSE
      To allocate memory from system.

   INPUT
		ppv :       Pointer to ptr to set to allocated memory.
		u32Size :   Size in bytes of memory needed. size is rounded up to nearest
                    word size of target CPU
		mst :        Memory type requested flags. 

   OUTPUT
		*ppv is set to address of allocated memory. *ppv set to NULL on failure.

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		08/15/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL fAllocMem_Private (void **ppv, UINT32 u32Size, MEMSAFETYPE mst)
BEGINFUNC (fAllocMem_Private)
{
   
   int i;
   UINT8 *pub;
   MEMPIECE *pmemp;

   pub = NULL;

   /* Round up size to nearest word */
   u32Size = (INT32)AlignedUpVal (u32Size);

   for (i = 0, pmemp = gmemsys.argmemscope[gmemsys.igmemscope].armemp; i < gmemsys.cPieces; i++, pmemp++)
   {
		 // This shows the size of the free memory before the request is fulfilled and the memory type of the free
		 // memory and the memory type of the request.
		 DebugMess(("F=%lu (0x%lX:0x%lX) ", (unsigned long)(SizeOfFreeGMemPiece(pmemp)) , mst, pmemp->mst));      

      // Is there enough memory for requested size plus magic cookies?
      if (SizeOfFreeGMemPiece (pmemp) >= (u32Size + 2 * sizeof(UINT32)) && MemTypeMatch(mst, pmemp->mst))
      {
         if (mst & mst_Temp) // Take from top of available memory instead of bottom.
         {
            pmemp->pu8MemEnd -=  sizeof(UINT32);
            *((UINT32 *)pmemp->pu8MemEnd) = u32Size;  // Write size just after end of allocated memory.
            pmemp->pu8MemEnd -= (u32Size + sizeof(UINT32));
            *((UINT32 *)pmemp->pu8MemEnd) = u32Size;  // Write size just before start of allocated memory.
            pub = pmemp->pu8MemEnd + sizeof(UINT32);
         }
         else
         {
            pub = pmemp->pu8MemStart;  // Get pointer to mem.
            pmemp->pu8MemStart += u32Size; // Make scope give up the mem.
            #if EL_DEBUG_MEMORY
               /*
               ** Place cookies that tells size of allocation so we can 
               ** verify adherence to last in first out rule when freeing 
               ** memory.
               */
               *((uint32 *)pub) = u32Size;   // place cookie at start.
               pub += sizeof (UINT32);       // advance user past cookie.
               pmemp->pu8MemStart += sizeof (UINT32); // Advance memstart for start cookie.
               *((UINT32 *)pmemp->pu8MemStart) = u32Size;   // place cookie at end.
               pmemp->pu8MemStart += sizeof (UINT32);       // advance memstart past end cookie.

            #endif
         }
         ENSURE (pmemp->pu8MemStart <= pmemp->pu8MemEnd);

         if (mst & mst_Fill) memset (pub, (int)(mst&0xFF), u32Size);
         break;
      }
   }
   *ppv = (void *)pub; 
   
	 // This shows the values for the allocated piece of memory: Start Address: End Adress, Size
	 DebugMess(("MEM:B=0x%lX E=0x%lX T=%lu\n", (unsigned long)pub, (unsigned long)(pmemp->pu8MemStart), u32Size));
   RETURN (pub != NULL);

} ENDFUNC (fAllocMem_Private)

/*************************************************************************
                         AllocMemNoFail_Private
 *************************************************************************

   SYNOPSIS
		void AllocMemNoFail_Private (void **ppv, UINT32 u32Size, MEMSAFETYPE mst)

   PURPOSE
		Same as AllocMem_Private except exits on failure

   INPUT
		ppv     :
		u32Size :
		mst     :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		12/06/95 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void AllocMemNoFail_Private (void **ppv, UINT32 u32Size, MEMSAFETYPE mst)
BEGINPROC (AllocMemNoFail_Private)
{
	if (!fAllocMem_Private (ppv, u32Size, mst))
	{
		FailureMess (("Out of Memory: Couldn't allocate %lu bytes\n", u32Size));
	}

} ENDPROC (AllocMemNoFail_Private)

/*************************************************************************
                             FreeMem_Private
 *************************************************************************

   SYNOPSIS
		void FreeMem_Private (void *pv)

   PURPOSE
      To free memory back to system.

   INPUT
		pv :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/15/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void FreeMem_Private (void *pv)
BEGINPROC (FreeMem_Private)
{
   MEMPIECE *pmemp, *pmempOuterScope;
   int i;
   UINT8 *pu8;

   ENSURE (gmemsys.igmemscope > 0);
   ENSURE (!IsUnAlignedPtr (pv));

   pu8 = (UINT8 *)pv;
   /*
   ** Find which piece this memory came from.
   */
   for (
      i = 0,
         pmemp = gmemsys.argmemscope[gmemsys.igmemscope].armemp,
         pmempOuterScope = gmemsys.argmemscope[gmemsys.igmemscope-1].armemp;
      i < gmemsys.cPieces;
      i++, pmemp++, pmempOuterScope++
   )
   {
      /* Is this the piece the memory came from? */
      if (pmempOuterScope->pu8MemStart <= pu8 && pu8 < pmempOuterScope->pu8MemEnd )
      {
         /* Is it temp memory allocated from the end instead of from the start */
         if (pu8 >= pmemp->pu8MemEnd)
         {
            UINT32 u32UserSize;

            ENSURE (pu8 == (pmemp->pu8MemEnd + sizeof(UINT32)));  // Better be last in first out with space for size-cookie.
            u32UserSize = *((UINT32 *)(pmemp->pu8MemEnd));        // Pick up size-cookie.
            ENSURE (u32UserSize != 0);                            // Better not be 0 size!
            ENSURE (!IsUnAlignedVal (u32UserSize));               // Better be word divisible size.
            pmemp->pu8MemEnd = pu8 + u32UserSize + sizeof(UINT32);
            ENSURE (u32UserSize == *((UINT32 *)(pu8 + u32UserSize)));   // Size-cookie before block better one at end!
         }  
         else
         {
            ENSURE (pu8 < pmemp->pu8MemStart);  // Better not be freeing unallocated memory.
            #if EL_DEBUG_MEMORY
               {
                  UINT32 u32UserSize;
                  /*
                  ** This assertion looks at size-cookie and checks that the 
                  ** pointer being freeed is that far back from free mem start 
                  ** to verify adherance to the last in first out rule.
                  */
                  pmemp->pu8MemStart -= sizeof (UINT32);
                  u32UserSize =  *((UINT32 *)pmemp->pu8MemStart);
                  ENSURE_ (pu8 == (pmemp->pu8MemStart - u32UserSize), "Memory freed out of last-in-first-out order\n");
                  pu8 -= sizeof(UINT32);
                  ENSURE_ (*((UINT32 *)pu8) == u32UserSize, "Size cookies are different\n");
               }
            #endif
            pmemp->pu8MemStart = pu8;
         }
         break;
      }
   }
   ENSURE (i < gmemsys.cPieces); // Better have found the piece!

} ENDPROC (FreeMem_Private)


#if EL_DEBUG_MEMORY

/*************************************************************************
                             fMemOK_Private
 *************************************************************************

   SYNOPSIS
		BOOL fMemOK_Private (void *pv)

   PURPOSE
      To check integrity of cookies.

   INPUT
		pv : pointer to memory allocated with fAllocMem_Private().

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      TRUE if everything is OK. FALSE if memory corrupted.

   SEE ALSO


   HISTORY
		08/22/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL fMemOK_Private (void *pv)
BEGINFUNC (fMemOK_Private)
{
   UINT8 *pu8;
   UINT32 *pu32UserSize1, *pu32UserSize2;

   ENSURE_F (!IsUnAlignedPtr (pv), ("Unaligned memory pointer: 0x%p", pv));

   pu8 = (UINT8 *)pv;
   pu32UserSize1 = (UINT32 *)(pu8 - sizeof(UINT32));

   ENSURE_F (!IsUnAlignedVal (*pu32UserSize1), ("Unaligned allocation size: %lu", (unsigned long)(*pu32UserSize1)));
   ENSURE_F (*pu32UserSize1 > 0, ("Allocation size cookie is corrupted."));

   pu8 += *pu32UserSize1;
   pu32UserSize2 = (UINT32 *)pu8;

   ENSURE_F (*pu32UserSize1 == *pu32UserSize2, ("Allocation size cookies do not match: %lu != %lu", 
                     (unsigned long)(*pu32UserSize1),
                     (unsigned long)(*pu32UserSize2)));

   RETURN TRUE;

} ENDFUNC (fMemOK_Private)

/*************************************************************************
                         PrintMemState_Private
 *************************************************************************

   SYNOPSIS
		void PrintMemState_Private (void)

   PURPOSE
      To print out the current free memory pieces.

   INPUT
		None

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/22/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void PrintMemState_Private (void)
BEGINPROC (PrintMemState_Private)
{
   MEMPIECE *pmemp, *pmempOuterScope;
   int i;
   UINT8 *pu8;

   ENSURE (gmemsys.igmemscope > 0);

   /*
   ** Find which piece this memory came from.
   */
   InfoMessAppend (("Memory Pieces:\n"));

   for (
      i = 0,
         pmemp = gmemsys.argmemscope[gmemsys.igmemscope].armemp,
         pmempOuterScope = gmemsys.argmemscope[gmemsys.igmemscope-1].armemp;
      i < gmemsys.cPieces;
      i++, pmemp++, pmempOuterScope++
   )
   {
      MEMSAFETYPE mst;
      UINT32 u32FreeNorm, u32FreeTemp, u32FreeTotal;

      mst = pmemp->mst;

      InfoMessAppend (("MemPiece[%02ld]: TOTAL 0x%08p...0x%08p = 0x%08lx = %7lu ; Type [%s%s%s%s]\n",
                  i, pmempOuterScope->pu8MemStart, pmempOuterScope->pu8MemEnd-1,
                  SizeOfFreeGMemPiece(pmempOuterScope), SizeOfFreeGMemPiece(pmempOuterScope), 
                  pszIf(mst & mst_Temp, "Temp "), 
                  pszIf(mst & mst_Audio, "Audio "),
                  pszIf(mst & mst_Video, "Video "),
                  pszIf(mst & mst_Fill, "Fill ")
         ));

      u32FreeNorm = (unsigned long)(pmemp->pu8MemStart - pmempOuterScope->pu8MemStart);
      u32FreeTemp = (unsigned long)(pmempOuterScope->pu8MemEnd - pmemp->pu8MemEnd);
      u32FreeTotal = (u32FreeNorm + u32FreeTemp);

      InfoMessAppend (("              -USED 0x%08lx + 0x%08lx = 0x%08lx = %7lu\n",
                  u32FreeNorm, u32FreeTemp, u32FreeTotal, u32FreeTotal
         ));
      InfoMessAppend (("              ----- ----------   ----------   ----------   -------\n"));
      InfoMessAppend (("              =FREE 0x%08p...0x%08p = 0x%08x = %7lu\n\n",
                  pmemp->pu8MemStart, pmemp->pu8MemEnd-1,
                  SizeOfFreeGMemPiece(pmemp), SizeOfFreeGMemPiece(pmemp)
         ));
   }

} ENDPROC (PrintMemState_Private)
#endif

/*************************************************************************
                              MemTypeMatch
 *************************************************************************

   SYNOPSIS
		static BOOL MemTypeMatch (MEMSAFETYPE mstReq, MEMSAFETYPE mstMem)

   PURPOSE
      To tell if the memory type satisfies the requested type.

   INPUT
		mstReq : Requested type of memory.
		mstMem : Type of memory being considered for match.

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      TRUE if requirements satisfied (match). FALSE otherwise.

   SEE ALSO


   HISTORY
		08/22/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static BOOL MemTypeMatch (MEMSAFETYPE mstReq, MEMSAFETYPE mstMem)
BEGINFUNC (MemTypeMatch)
{
   RETURN  ((mstReq & mst_Any) || (( (mstReq & ~(0xFF)) & mstMem) == mstReq));

} ENDFUNC (MemTypeMatch)

