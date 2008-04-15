/*************************************************************************
 *                                                                       *
 *                              TMEMSAFE.C                               *
 *                                                                       *
 *************************************************************************

		Copyright (c) 1996-2008, Echidna

		All rights reserved.

		Redistribution and use in source and binary forms, with or
		without modification, are permitted provided that the following
		conditions are met:

		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer. 
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer
		  in the documentation and/or other materials provided with the
		  distribution. 

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
		CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
		MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
		DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
		BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
		TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
		DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
		ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
		OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
		POSSIBILITY OF SUCH DAMAGE.

   DESCRIPTION
      Tools Memory Management subssytem to memsafe.c

   PROGRAMMERS
		John M. Alvarado, Gregg Tavares

   FUNCTIONS

   TABS : 4 7

   HISTORY
		07/11/96 GAT: Created

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


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/

static BOOL MemTypeMatch (MEMSAFETYPE mstReq, MEMSAFETYPE mstMem);

/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/



/*************************************************************************
                           fOpenMemSys_Private
 *************************************************************************

   SYNOPSIS
		BOOL fOpenMemSys_Private (UINT32 u32SizeSpared, UINT32 u32SizeHid)

   PURPOSE
      To initialize tools memory system.

      This function should be accessed only through macros defined in 
      memsafe.h.  Do not call directly.

   INPUT
		u32SizeSpared: Not used.
      u32SizeHid:    Not Used.

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
   WarningMess (("Memory scopes not supported in this system."));
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
   WarningMess (("Memory scopes not supported in this system."));

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
		None

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
   UINT8 *pub;

   pub = (UINT8 *)malloc (u32Size);
   if (pub)
   {
      if (mst & mst_Fill)
      {
          memset (pub, (int)(mst&0xFF), u32Size);
      }
   }

   *ppv = (void *)pub;

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
		FailureMess (("Out of Memory: Couldn't allocate %ld bytes\n", u32Size));
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
   free (pv);
} ENDPROC (FreeMem_Private)


#if EL_DEBUG_MEMORY

/*************************************************************************
                             fMemOK_Private
 *************************************************************************

   SYNOPSIS
		BOOL fMemOK_Private (void *pv)

   PURPOSE
      To check integrity of memory.

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
   /*
   ** Depending on the system this function could check to make sure the 
   ** pointer is not pointing into obviously bad areas where it could not 
   ** have allocated the memory from.
   */

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
   RETURN  ((mstReq & mst_Any) || ((mstReq & mstMem) == mstReq));

} ENDFUNC (MemTypeMatch)


