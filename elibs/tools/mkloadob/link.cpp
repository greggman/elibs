/*=======================================================================*
 |   file name : link.cpp
 |-----------------------------------------------------------------------*
 |   function  : loading data and resolving pointers
 |-----------------------------------------------------------------------*
 |   author    : Gregg Tavares
 |-----------------------------------------------------------------------*

    This is data linker code for one chunk files.  A typical arguments for
    a one chunk file would be

    mkloadob mylevel.lbi -duperr -sectorsize 2048 -chunksize $2800000
         -nopad -nosort -fixupmode 2 -padsize 4 linkerfile.dlk

    If you are targeting a bigendian machine at the option "-bigendian"

 |-----------------------------------------------------------------------*

	The Echidna Copyright
	
	Copyright 1991-2003 Echidna, Inc. All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:
	
	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	
	THIS SOFTWARE IS PROVIDED BY Echidna ``AS IS'' AND ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
	NO EVENT SHALL Echidna OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	
	The views and conclusions contained in the software and documentation are
	those of the authors and should not be interpreted as representing
	official policies, either expressed or implied, of Echidna or
	Echidna, Inc.

 *=======================================================================*/

/**************************** i n c l u d e s ****************************/

#include <nldef.h>
#include <assert.h>

#include "link.h"
#include "quickload.h"

/*************************** c o n s t a n t s ***************************/

#define LINK_OFFSET_SHIFT	0
#define	LINK_OFFSET_MASK	0x03FFFFFC
#define LINK_BLOCK_SHIFT	26

#define	LINKBIT_IS_NOTRESOLVED	0x1
#define LINKBIT_IS_RUNTIMEFILE	0x2

/******************************* t y p e s *******************************/


/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/

#define LINK_GET_OFFSET(v)		((((Uint32)(v)) & LINK_OFFSET_MASK) << LINK_OFFSET_SHIFT)
#define LINK_GET_BLOCK(v)		(((Uint32)(v)) >> LINK_BLOCK_SHIFT)
#define LINK_IS_NOTRESOLVED(v)	(((Uint32)(v)) & LINKBIT_IS_NOTRESOLVED)
#define LINK_IS_RESOLVED(v)		(!(((Uint32)(v)) & LINKBIT_IS_NOTRESOLVED))
#define LINK_IS_FILE(v)			(((Uint32)(v)) & LINKBIT_IS_RUNTIMEFILE)

#if 0
	#define LINK_RESOLVE(blktbl,v)	(void *)(((Uint32*)blktbl)[LINK_GET_BLOCK(v)]+LINK_GET_OFFSET(v))
#else
	static void *LINK_RESOLVE(Uint32* blktbl, Uint32 value)
	{
		Uint32 offset    = LINK_GET_OFFSET(value);
		Uint32 blockndx  = LINK_GET_BLOCK(value);
		Uint32 blockaddr = blktbl[blockndx];
		Uint32 result    = blockaddr + offset;
		
		return (void *)result;
	}
#endif

/**************************** r o u t i n e s ****************************/

/*************************************************************************
                           LINK_GetBundleStart
 *************************************************************************

   SYNOPSIS
		void* LINK_GetBundleStart (void* blocks)

   PURPOSE
  		Return a pointer to the [START] section of a bundle.
        You must have already called LINK_InitBundle!

   INPUT
		blocks : pointer to start of memory for bundle

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
        pointer to [START] section

   SEE ALSO


   HISTORY
		12/13/02 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void* LINK_GetBundleStart (void* blocks)
{
	Uint32* walk = (Uint32 *)blocks;
	
	// walk the block table
	while (*walk)
	{
		walk++;
	}
	
	// skip the marker
	walk++;
	
	// the next value is a pointer to the start
	Uint32 start;

	start = *walk;

    assert (!LINK_IS_NOTRESOLVED(start));

    return (void*)start;
}

/*************************************************************************
                           LINK_FixupPointers
 *************************************************************************

   SYNOPSIS
		void* LINK_FixupPointers (void* block)

   PURPOSE
  		Given a pointer to block zero of a set of blocks,
		resolves all the pointers assuming it's the kind of
		mkloadob with fixup links at the end of the file

        It is SAFE to call this function more than once even IF you've
        freed the fixup table after calling it at least once.

   INPUT
		blocks :
        freeFromHere : pointer to void* that gets filled in with
                        start of memory which you may free (fixuptable),

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		05/17/02 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void* LINK_FixupPointers (void* blocks, void** freeFromHere)
{
	bool fFreeable   = true;
	
	Uint32* blocktable = (Uint32 *)blocks;
	Uint32* walk = blocktable;
    Uint32* pStart;
	void* freeHere;
	
	// walk the block table
	while (*walk)
	{
		walk++;
	}
	
	// skip the marker
	walk++;
	
	// the next value is a pointer to the start
	Uint32 start;

    pStart = walk;	
	start = *walk++;

    // only do this if we have not already done it
    if (LINK_IS_NOTRESOLVED(start))
    {
        // compute start AND save it
        start = (Uint32)LINK_RESOLVE (blocktable, start);
        *pStart = start;
	
    	// the next file is a pointer to the fixup table
    	Uint32* fixuptable;
    	
    	fixuptable = (Uint32*)LINK_RESOLVE (blocktable, *walk);
    	freeHere   = fixuptable;
    	
    	// walk the fixup table
    	while (*fixuptable)
    	{
    		Uint32* pntr2pntr;
    		Uint32  fixpntr;
    		
    		pntr2pntr = (Uint32*)(*fixuptable);
    		pntr2pntr = (Uint32*)LINK_RESOLVE (blocktable, (Uint32)pntr2pntr);
    		
    		fixpntr   = *pntr2pntr;
    		
    		if (LINK_IS_FILE (fixpntr))
    		{
    			Uint32*	filepntr;
    			
    			fFreeable = false;
    			
    			// this is a file, follow the pointer
    			filepntr = (Uint32*)LINK_RESOLVE (blocktable, fixpntr);
    			
    			// is the file loaded?
    			if (!*filepntr)
    			{
    				// loadfile
    				void* filedata = QuickLoadFile (((char*)filepntr) + 4);
    				
    				*filepntr = (Uint32)filedata;
    			}
    			
    			*pntr2pntr= *filepntr;
    		}
    		else if (LINK_IS_NOTRESOLVED(fixpntr)) // I don't think I need this, I think it will always be unresolved
    		{
    			fixpntr    = (Uint32)LINK_RESOLVE (blocktable, fixpntr);
    			*pntr2pntr = fixpntr;
    		}
    		
    		fixuptable++;
    	}

    	// if no files were loaded we can free from here
        if (freeFromHere)
        {
        	*freeFromHere = (fFreeable ? freeHere : NULL);
        }
    }
    	
	// return a pointer to the first thing
	return (void*)start;
}

/*************************************************************************
                             LINK_InitBundle
 *************************************************************************

   SYNOPSIS
		void* LINK_InitBundle (void* data, void** freeFromHere)

   PURPOSE
  		Given a pointer to a linked bundle file that is setting in memory,
        fixes up all the pointers and returns a pointer to the [start]
        section

   INPUT
		data :
        freeFromHere: (fixup table)

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
        pointer to [start] section

   SEE ALSO


   HISTORY
		12/11/02 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void* LINK_InitBundle (void* data, void** freeFromHere)
{
    // WE ARE ASSUMING 1 BLOCK HERE!!!
    *((void**)data) = data;

    return LINK_FixupPointers (data, freeFromHere);
}

/*************************************************************************
                         LINK_QuickLoadBlockFile
 *************************************************************************

   SYNOPSIS
		void* LINK_QuickLoadBlockFile (char* filename, void** freePntr)

   PURPOSE
  		Load a block file, resolve the pointers

        ********************************************************
        ***                                                  ***		
		*** !!! DO NOT USE THIS FUNCTION IN RELEASE CODE !!! ***
        ***                                                  ***		
        ********************************************************

   INPUT
		filename : file to load
        freePntr : pointer to start of block of memory that was allocated (optional)

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		05/17/02 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void* LINK_QuickLoadBlockFile (const char* filename, void** freePntr)
{
	void* blockfile;
	void* freeableMem;
	
	blockfile = QuickLoadFile (filename);
	if (blockfile)
	{
        if (freePntr)
        {
            *freePntr = blockfile;
        }

        return LINK_InitBundle (blockfile, &freeableMem);
	}
	
	return NULL;
}


