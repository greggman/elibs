/*
 * MKLOADOB.C
 *
 *  COPYRIGHT : 1993 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 05/09/93
 *   MODIFIED : 09/08/94
 *       TABS : 05 09
 *
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Takes a bunch of spec files of the form below and generates
 *		a hierarchical binary file with pointers , optionally checks for
 *		duplicates and creates a relative offset table for each
 *		piece of data.  Will separate data into chucks of a specified size
 *		If you don't need chucks specify a really large size and you'll get
 *		just one chunk!  Note: no piece of data or any tables can be larger
 *		than a single chunk.
 *
 *      What's the point?
 *
 *      1) You want to spool music from the CD and STILL be able to load
 *         in real time.  Load one chuck at a time (giving the machine
 *		   time to grab more music between chucks
 *
 *      2) You want to be able to LOAD and later FREE multiple levels
 *         in the same memory space without having to worry about
 *         garbage collection and fragmination.  You do this by storing
 *         the level in chunks using this program.
 *
 *         At runtime you allocation say 200 chunks of memory.  You can
 *         then load each chunk from on of these files into any of those
 *         200 chunks in any order since all pointers are chuck relative.
 *
 *         The beginning of the file contains a pointer field for each
 *         chunk that ends in NULL.  So if there are 4 chunks the start
 *         of the very first chunk would be
 *
 *         $FFFFFFFF	; space for pointer for chunk 0
 *         $FFFFFFFF	; space for pointer for chunk 1
 *         $FFFFFFFF	; space for pointer for chunk 2
 *         $FFFFFFFF	; space for pointer for chunk 3
 *         $00000000	; end marker
 *		   $????????	; pointer to start of data
 *
 *		   As you load each chunk you can record it's address in memory
 *         in that table.
 *
 *		   When you are finish you can go and fix up all the pointers in
 *		   your data something like this
 *
 *		   long* chunktable;   : address of first chunk
 *         long  relpntr;      : a pointer you've gotten out of the data
 *         void* pntr;         : actuall address in memory
 *
 *		   pntr = chunktable[relpntr & (LOAD_CHUNK_MASK >> LOAD_CHUNK_SHIFT)] + (relpntr & LOAD_CHUCK_OFFSET_MASK)
 *
 *		   It is assumed that no pointer may be at an ODD address
 *		   therefore all pointers in the file have their low bit set
 *         (0x00000001).  This is so as you fix up the pointers you
 *         know if the pointer has already been fixed up.  This is
 *         important because if you are fixing up the pointers by
 *         walking the data yourself.
 *
 *         so for example if you have this
 *
 *         [start]
 *         pntr=section1
 *         pntr=section1
 *
 *         [section1]
 *         file=myfile.bin
 *
 *		   As you walk your data you will encounter 2 pointers to section1.
 *         When you find the first one you would go fix up section1.
 *         When you find the second one you would go to fix up section1
 *         and if you didn't know the pointers were already fixed up
 *         you'd fix them again and mess them up.
 *
 *         Optionally you can write out a fixup table.  This is a list of
 *         relative pointers too ALL other pointers in the file followed by
 *		   a null/0
 *
 *         There are currently two problems with this method.
 *
 *         1) The fixup pointers must all fit in the first chunk
 *
 *         2) the fixup pointers are included in the file so that
 *            it would be hard to use the memory they take up after
 *            you are done with them.
 *
 * NOTES
 *      *) When using the multiple chunk method each chunk is padded to
 *      be one full chunk long.  If you are only using one chunk
 *      you probably want to use -NOPAD option to not pad since you will
 *      specify an arbitarily large chunk.
 *
 *      BUT, at the same time you probably want your in game loader
 *      to be as simple as possible like for example never having to
 *      load less than a full sector of data from the CD/DVD.  In that
 *      case use the -SECTORSIZE option to specify the size of a sector on the CD/
 *      DVD.  This is pretty much only useful if you are using ONE chunk.
 *      
 *      Why is that important? Because many DVD/CD libraries will read
 *		direclty to user memory if they can read an entire sector but if
 *      they have to read less than an entire sector they must first
 *      load it to their own buffer then copy the part you wanted to
 *      user memory.  That's clearly slower than directly reading it.
 *
 *		*) Loading a multi-chunk file would look something like this:
 *	
 *			Get length of entire file and divide by the chunk size
 *			to get the number of chunks
 *		
 *			for each chunk
 *	
 *				Load a chunk, record the address of the loaded chunk
 *				in the corresponding long word in the first chunk.
 *	
 *			endfor
 *	
 *			//
 *			// now all the blocks are loaded and you know where the
 *			// data offset table starts so
 *			//
 *	
 *			for each non-zero data offset
 *	
 *				look up the correct block address
 *				add the block offset
 *				store in the data offset
 *	
 *			endfor
 *	
 *      Preprocessor commands:
 *
 *          #include "filename" ; includes another linker file
 *          #define var=value   ; defines a var that can be references as %var%
 *
 *          note: these are implemented in the ReadINI module in the echidna libs
 *
 * 		Commands:
 *
 *          IMPORTANT!!!
 *
 *          data fields are not automatically aligned.  In other words
 *
 *          [mysection]
 *          byte=1
 *          long=2
 *
 *          Will result in a long starting at a 1 byte offset from the
 *          start of mysection
 *
 *          The reason is this allows you to use the data linker to make
 *          unaligned data.  If you want aligned data you need to use
 *          the align= option
 *
 *			file=			; pointer to a file						: quotes are optional
 *			data=			; pointer to a file (same as file=)		: quotes are optional
 *			load=			; pointer to a file loaded at runtime	: quotes are optional 
 *									
 *                            note: this will actually start as a pointer to a filename with
 *                                  with the POSITIONFLAG_ISFILE set to tell your loader that
 *                                    you need to load the file 
 *									
 *									since mkloadob excepts variables both as environment vars
 *									and as #define var=value, the expected usage of load= is as follows
 *									
 *										#define objectload=load
 *										;#define objectload=file
 *										#define weaponsload=load
 *										;#define weaponsload=file
 *										
 *										%objectload%="myobject1.ob"
 *										%objectload%="myobject2.ob"
 *										%weaponload%="myweapon1.foo"
 *										%weaponload%="myweapon2.foo"
 *									
 *									this way you can comment turn on/off individual files
 *									or file types throughout.
 *
 *			pntr=			; pointer to another section
 *			level=			; pointer to another section (same as level=)
 *
 *			long=			; longs (4 bytes each)
 *			int32=			; longs (4 bytes each)
 *			uint32=			; longs (4 bytes each)
 *			word=			; words (2 bytes each)
 *			short=			; words (2 bytes each)
 *			int16=			; words (2 bytes each)
 *			uint16=			; words (2 bytes each)
 *			byte=			; bytes (1 byte each)
 *			char=			; bytes (1 byte each)
 *			int8=			; bytes (1 byte each)
 *			uint8=			; bytes (1 byte each)
 *			float=			; floats (4 bytes each)
 *			string=			; string, NO NULL IS INSERTED!!!
 *							;    string=ABC inserts ABC,
 *							;    string="ABC" inserts "ABC" (including the quotes!!!)
 *			binc=           ; binary file to include here
 *							;    quotes are optional
 *			align=			; boundry to align to (no arg or 0 = default)
 *                          ; NOTE: THIS ALIGNS BASED ON THE START OF THE SECTION
 *                          ;       NOT MEMORY.
 *                          ;      
 *                          ;     In otherwords, if you have section like this
 *                          ;      
 *                          ;         [mysection]
 *                          ;         byte=1
 *                          ;         align=4
 *                          ;         long=$12345678
 *                          ;         
 *                          ;      You will get 3 bytes of padding after the first byte BUT
 *                          ;      the section will be placed based on the -PADSIZE option.
 *                          ;      If -PADSIZE is set to a non multiple of 4 in the example
 *                          ;      above it's possible the section will not start at a 4 byte
 *                          ;      boundry and therefore neither will the long
 *                          ;         
 *          pad=            ; pad with N bytes
 *                          ;
 *                          ;           pad=24 ; insert 24 bytes (value 0)
 *                          ;
 *          secalign=       ; align the start of the current section
 *                          ; can appear anywhere in the section so for example
 *                          ;
 *                          ;         [mysection]
 *                          ;         byte=1
 *                          ;         long=$12345678
 *                          ;         secalign=16
 *                          ;
 *                          ;      My section will start at a 16 byte boundary
 *                          ; 
 *                          ; NOTE: the boundry is relative to the address of the particular
 *                          ; block this section ends up being placed in.  The boundry of
 *                          ; block is up the loader in the program you are using this data.
 *                          ;
 *			path=			; set the path for loading binary files		 
 *							; files encountered after this line will be loaded from here
 *							; NOTE: Files are parsed from the TOP section to each connecting section
 *							;       so for example
 *
 *							[start]
 *							pntr=section1
 *							file=file3.bin		; load dir\subdir\file2.bin
 *							
 *							[section1]
 *							path=dir\subdir
 *							file=file1.bin		; loads dir\subdir\file1.bin
 *
 *          insert=         ; inserts another section here (not a pointer, the actual section
 *                          ; so for example this:
 *
 *                          [start]
 *                          byte=1 2 3
 *                          insert=othersection
 *                          byte=7 8 9
 *
 *                          [othersection]
 *                          byte=4
 *                          byte=5 6
 *
 *                          ; is the same as this
 *
 *                          [start]
 *                          byte=1 2 3
 *                          byte=4
 *                          byte=5 6
 *                          byte=7 8 9
 *							
 *
 *
 *
 * HISTORY
 *	
 * TODO
 *
 *		* Add Best Fit Algorithm
 *
 *
 *
*/

#include "platform.h"
#include "switches.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <echidna/utils.h>
#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/datafile.h>
#include <echidna/eio.h>
#include <echidna/strings.h>
#include <echidna/listapi.h>
#include <echidna/checkglu.h>
#include <echidna/readini.h>
#include <echidna/hash.h>

/**************************** C O N S T A N T S ***************************/

#define	PRELOAD_VERSION	0x01010101

#define MAX_LINE	    1024
#define	MAX_ARGS	    128
#define MAX_NAME_LEN    1024

#define	BUF_SIZE	(512*1024)

#define POSITIONFLAG_UNRESOLVED	0x1
#define POSITIONFLAG_ISFILE		0x2
#define POSITIONFLAG_BITMASK    0x03

/******************************** T Y P E S *******************************/

typedef struct NamedPair
{
    LST_NODE    node;
    char*       value;
} NamedPair;

typedef struct NamedPairs
{
    LST_LIST*   pairList;
    LST_LIST    pairListX;
}
NamedPairs;

typedef struct PreLoadFile
{
	LST_NODE			node;
	long				size;
	uint8*				data;
	char*				sameAs;
	struct PreLoadFile*	sameAsFile;
}
PreLoadFile;

typedef struct
{
	LST_NODE		node;
	LST_LIST		partsListX;
	LST_LIST*		partsList;
	long			numParts;
	long			size;
    long            alignment;  // non zero = align to boundary
	struct FileContents	*fc;
}
Level;

typedef struct FileContents
{
	LST_NODE	 Node;
	uint8*		 Data;
	int			 bFromPreLoad;
	int			 bLoadAtRuntime;
	long		 Size;
	long		 Offset;
	long		 PadSize;
    long         Alignment;
	struct FileContents	*SameAs;
	Level		*Level;

	uint32		 hashValue;
	BOOL		 bHasHashValue;
}
FileContents;

typedef struct
{
	LST_NODE		 Node;
	FileContents	*fc;
}
PositionNode;

#define PART_DATA			1
#define PART_LEVEL			2
#define	PART_LONG			3
#define	PART_WORD			4
#define	PART_BYTE			5
#define PART_STRING			6
#define	PART_PATH			7
#define PART_FLOAT			8
#define	PART_BINC			9
#define PART_RUNTIMEFILE	10
#define PART_ALIGN          11
#define PART_PAD            12

typedef struct
{
	LST_NODE		 node;
	long			 type;
	char			*string;
	FileContents	*fc;
	Level			*level;
	long			 size;
}
Part;

/****************************** G L O B A L S *****************************/

int				 DontOut      = FALSE;
int				 Verbose      = FALSE;
int				 Pack         = TRUE;
int				 OutMode	  = 0;
int				 UseFixupsMode= 0;
int				 PadEnd		  = TRUE;
int				 fDontSort    = FALSE;
int              fDupErr      = FALSE;
long			 PadSize	  = 4;
long			 ChunkSize    = 2048;
long			 HardwareSectorSize = 1;
int				 LittleEndian = TRUE;
long			 BytesWritten = 0;
long			 EndPadBytes  = 0;
long			 HardwarePadBytes = 0;
long			 PositionOffsetShift = 0;	// amount to shift the offset
long			 PositionBlockShift = 0;    // amount to shift the position
long			 MaxBlocks = 0;				// max number of blocks/chunks we can handle because of space in the position

long			 blocksMarked;

HashTable*		 FileContentsTable;
HashTable*		 PreLoadTable;
HashTable*		 FileTable;

LST_LIST		 PreLoadFileListX;
LST_LIST		*PreLoadFileList = &PreLoadFileListX;

LST_LIST		 FileListX;
LST_LIST		*FileList = &FileListX;

LST_LIST		 PosListX;
LST_LIST		*PosList = &PosListX;

LST_LIST		 SortedListX;
LST_LIST		*SortedList = &SortedListX;

LST_LIST		 LevelListX;
LST_LIST		*LevelList = &LevelListX;

long			 guessSize;
long			 TotalFiles;
long			 TotalFixups;

char			 line[MAX_LINE];
char			*args[MAX_ARGS];

char			*inputPath = "";

char			*topSection = "Start";

/******************************* M A C R O S ******************************/

/******************************* T A B L E S ******************************/

uint8			 ZeroData[256] = { 0, };

/***************************** R O U T I N E S ****************************/

Level* ParseLevel (IniList *specFile, char *levelName, ConfigLine* pCL);

const char* LocalGetConfigFilename (ConfigLine* pCL)
{
    return pCL != NULL ? GetConfigFilename (pCL) : "*mainfile*";
}

int LocalGetConfigLineNo (ConfigLine* pCL)
{
    return pCL != NULL ? GetConfigLineNo (pCL) : 0;
}

long roundUp (long value, long padsize)
{
    if (value % padsize)
    {
        value = value + padsize - value % padsize;
    }
    
    return value;
}


/*************************************************************************
                            NP_SplitNamedPairs                           
 *************************************************************************

   SYNOPSIS
		NamedPairs* NP_SplitNamedPairs (const char* str, char pairSplit, char valueSplit)

   PURPOSE
  		Take a string like this "foo=1,goo=boof" and parse it so it's easy to use
  
   INPUT
		str        :
		pairSplit  : value to split pairs by (eg, ',')
		valueSplit : value to splut value and label by (eg, '=')
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
        NamedPairs pointer, used to pass to other NP_ funcs
  
   SEE ALSO
  
  
   HISTORY
		01/24/03 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

NamedPairs* NP_SplitNamedPairs (const char* str, char pairSplit, char valueSplit)
{
    NamedPairs* pNP = CHK_CallocateMemory (sizeof (NamedPairs), "NamedPairs");
    
    pNP->pairList = &pNP->pairListX;
    LST_InitList (pNP->pairList);
    
    {
        while (*str)
        {
			const char* pairStart;
			const char* valueSplitter = NULL;

            while (*str && isspace(*str)) ++str;
            
            pairStart = str;
            
            while (*str && *str != pairSplit)
            {
                if (*str == valueSplit)
                {
                    valueSplitter = str;
                }
				++str;
            }
            
            if (pairStart != str)
            {
                BOOL fHaveValue = (valueSplitter != NULL && valueSplitter + 1 != str);
                const char* nameEnd  = (valueSplitter != NULL) ? valueSplitter : str;
                const char* valueStart = fHaveValue ? valueSplitter + 1 : NULL; 
                const char* valueEnd   = fHaveValue ? str : NULL;
                
                // eat whitespace after name
                while (nameEnd > pairStart && isspace (nameEnd[-1])) --nameEnd;
                
                // eat whitespace before and after value
                if (valueStart)
                {
                    while (valueStart < valueEnd && isspace(*valueStart)) ++valueStart;
                    while (valueEnd > valueStart && isspace(valueEnd[-1])) --valueEnd;
                }
                
                {
                    int nameLen =  nameEnd - pairStart;
                    int valueLen = valueEnd - valueStart;
                    
                    char* name  = CHK_AllocateMemory (nameLen + 1, "namedpair name");
                    char* value = CHK_AllocateMemory (valueLen + 1, "namedpair value");
    
                    // copy name                
                    strncpy (name, pairStart, nameLen);
                    name[nameLen] = '\0';
                    
                    // copy value
                    if (valueStart)
                    {
                        strncpy (value, valueStart, valueLen);
                        value[valueLen] = '\0';
                    }
                    else
                    {
                        value = CHK_dupstr ("1");
                    }
                    
					{
						NamedPair* pPair = CHK_CreateNode (sizeof (NamedPair), name, "namedpair");
						pPair->value = value;
                    
						LST_AddTail (pNP->pairList, pPair);
					}
                    
                    CHK_DeallocateMemory (name, "namedpair name");
                }
            }
            
            // skip pairSplit
            if (*str) ++str;
        }
    }
    
    return pNP;
}

/*************************************************************************
                           NP_GetValueForName                            
 *************************************************************************

   SYNOPSIS
		BOOL NP_GetValueForName (NamedPairs* pNP, const char* name, char** pValue)

   PURPOSE
  		find a named value
  
   INPUT
		pNP    : NamedPairs pointer from NP_SplitNamedPairs
		name   : name we want (not case sensitive)
		pValue : pointer to pointer to char for value
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
        True if name exists, false if not
  
   SEE ALSO
  
  
   HISTORY
		01/24/03 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL NP_GetValueForName (NamedPairs* pNP, const char* name, char** pValue)
{
    NamedPair* pPair = (NamedPair*)LST_FindIName (pNP->pairList, name);
    if (pPair)
    {
        *pValue = pPair->value;
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
                                 NP_Free                                 
 *************************************************************************

   SYNOPSIS
		void NP_Free (NamedPairs* pNP)

   PURPOSE
  		Free this stuff
  
   INPUT
		pNP :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		01/24/03 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void NP_Free (NamedPairs* pNP)
{
    NamedPair* pPair = (NamedPair*)LST_Head (pNP->pairList);
    while (!LST_EndOfList (pPair))
    {
        CHK_freestr (pPair->value);
        pPair = (NamedPair*)LST_Next (pPair);
    }
    
    LST_EmptyList (pNP->pairList);
    CHK_DeallocateMemory (pNP, "namedpairs");
}


/*************************************************************************
                               hashGetLong                               
 *************************************************************************

   SYNOPSIS
		char* hashGetLong (char *s, uint32 *l)

   PURPOSE
  		
  
   INPUT
		s :
		l :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

char* MK_hashGetLong (char *s, uint32 *l)
{
	uint32	v[4];
	int		i;

	for (i = 0; i < 4; i++)
	{
		v[i] = (*s) ? tolower(*s) : 0;
		s   += (*s) ? 1  : 0;
	}

	*l = ((v[3] << 24) | (v[2] << 16) | (v[1] << 8) | (v[0]));

	return s;
}

/*************************************************************************
                            LST_NodeHashFunc                             
 *************************************************************************

   SYNOPSIS
		uint32 LST_NodeHashFunc (HashEntry *he)

   PURPOSE
  		
  
   INPUT
		he :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

uint32 LST_NodeHashFunc (HashEntry *he)
{
	uint32	hash = 0;
	uint32	val  = 0;
	char	*s;

	s = LST_NodeName (he->data);
	while (*s)
	{
		s    = MK_hashGetLong (s, &val);
		hash = hash ^ val;
	}

	return hash;
}

/*************************************************************************
                           LST_NodeHashCmpFunc                           
 *************************************************************************

   SYNOPSIS
		int LST_NodeHashCmpFunc (HashEntry *t1, HashEntry *t2)

   PURPOSE
  		
  
   INPUT
		t1 :
		t2 :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int LST_NodeHashCmpFunc (HashEntry *t1, HashEntry *t2)
{
	return (stricmp (LST_NodeName (t1->data), LST_NodeName(t2->data)));
}

/*************************************************************************
                            FileContentsHashFunc                             
 *************************************************************************

   SYNOPSIS
		uint32 FileContentsHashFunc (HashEntry *he)

   PURPOSE
  		
  
   INPUT
		he :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

uint32 FileContentsHashFunc (HashEntry *he)
{
	uint32	hash = 0;
	uint32	val  = 0;
	FileContents*	fc;

	fc = (FileContents*)he->data;
	
	if (fc->bHasHashValue)
	{
		return fc->hashValue;
	}
	
	if (Pack)
	{
		uint8*	data = fc->Data;
		long	size = fc->Size - (fc->Size % 4);
	
		while (size)
		{
			hash += ((uint32)data[0]      ) +
					((uint32)data[1] <<  8) +
					((uint32)data[2] << 16) +
					((uint32)data[3] << 24) ;
			data += 4;
			size -= 4;
		}
		
		fc->hashValue     = hash;
		fc->bHasHashValue = TRUE;
	}
	else
	{
		static int bogusHashValue;
		
		fc->hashValue     = ++bogusHashValue;
		fc->bHasHashValue = TRUE;
	}

	return hash;
}

/*************************************************************************
                           FileContentsHashCmpFunc                           
 *************************************************************************

   SYNOPSIS
		int FileContentsHashCmpFunc (HashEntry *t1, HashEntry *t2)

   PURPOSE
  		
  
   INPUT
		t1 :
		t2 :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int FileContentsHashCmpFunc (HashEntry *t1, HashEntry *t2)
{
	if (((FileContents*)t1->data)->Size != ((FileContents*)t2->data)->Size)
	{
		return TRUE;
	}
	
	return (
			memcmp (
			((FileContents*)t1->data)->Data,
			((FileContents*)t2->data)->Data,
			((FileContents*)t2->data)->Size
			)
			);
}

/*************************************************************************
                             FindPreLoadFile                             
 *************************************************************************

   SYNOPSIS
		PreLoadFile* FindPreLoadFile (char* name)

   PURPOSE
  		
  
   INPUT
		name :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PreLoadFile* FindPreLoadFile (char* name)
{
	static	PreLoadFile	findPLF;
	
	PreLoadFile*	pPLF = NULL;
	HashEntry		find;
	HashEntry*		he;
	
	LST_NodeName(&findPLF) = name;
	find.data = &findPLF;
	
	he = HASH_FindHashEntry (PreLoadTable, &find);
	
	if (he != NULL)
	{
		pPLF = (PreLoadFile*)he->data;
	}

	return pPLF;
}

/*************************************************************************
                             FindFileContentsByName                            
 *************************************************************************

   SYNOPSIS
		FileContents* FindFileContentsByName (char* name)

   PURPOSE
  		
  
   INPUT
		name :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

FileContents* FindFileContentsByName (char* name)
{
	static	FileContents	findFC;
	
	FileContents*	pFC = NULL;
	HashEntry		find;
	HashEntry*		he;
	
	LST_NodeName(&findFC) = name;
	find.data = &findFC;
	
	he = HASH_FindHashEntry (FileTable, &find);
	
	if (he != NULL)
	{
		pFC = (FileContents*)he->data;
	}

	return pFC;
}

/*************************************************************************
                        FindFileContentsByContent                        
 *************************************************************************

   SYNOPSIS
		FileContents* FindFileContentsByContent (FileContents* fc)

   PURPOSE
  		
  
   INPUT
		fc :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

FileContents* FindFileContentsByContent (FileContents* fc)
{
	FileContents*	pFC = NULL;
	HashEntry		find;
	HashEntry*		he;
	
	find.data = fc;
	he = HASH_FindHashEntry (FileContentsTable, &find);
	
	if (he != NULL)
	{
		pFC = (FileContents*)he->data;
	}

	return pFC;
}

/*************************************************************************
                            AddFileToFileList                            
 *************************************************************************

   SYNOPSIS
		void AddFileToFileList (FileContents* fc)

   PURPOSE
  		
  
   INPUT
		fc :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void AddFileToFileList (FileContents* fc)
{
	LST_AddTail (FileList, fc);
	TotalFiles++;
	
	{
		HashEntry	*he;
	
		he = CHK_CallocateMemory (sizeof (HashEntry), "Hash Entry");
		he->data = fc;
	
		HASH_AddHashEntry (FileTable, he);
	}
}

long FilePosition (FileContents *fc)
{
	return ((fc->SameAs) ? fc->SameAs->Offset : fc->Offset);
}

long MK_Write (int fh, void *data, long len)
{
	BytesWritten += len;

	return CHK_Write (fh, data, len);
}

/*********************************************************************
 *
 * WritePosition
 *
 * SYNOPSIS
 *		void  WritePosition (int fh, long position, int fIsFile char *msg)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void WritePosition (int fh, long position, int fIsFile, char *msg)
{

	uint32	pos;
	long	block;
	long	offset;

	block    = position / ChunkSize;
	offset   = position % ChunkSize;

	pos = (offset >> PositionOffsetShift) | (block << PositionBlockShift) | POSITIONFLAG_UNRESOLVED | (fIsFile ? POSITIONFLAG_ISFILE : 0);

	if (Verbose)
	{
		EL_printf ("Block $%04lx : Offset $%06lx : %s\n", block, offset, msg);
	}

	{
		if (LittleEndian)
		{
			MakeLilLong (pos);
		}
		else
		{
			MakeBigLong (pos);
		}
		MK_Write (fh, &pos,  sizeof (uint32));
	}
}
// WritePosition

/*************************************************************************
                              AddLoadedFile                              
 *************************************************************************

   SYNOPSIS
		FileContents* AddLoadedFile (char *filename, long alignmentvoid* data, long len)

   PURPOSE
  		Add a file that's already been loaded
  
   INPUT
		filename :
        alignment:
		data     :
		len      :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/16/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

FileContents* AddLoadedFile (char *filename, long alignment, uint8* buf, long size, int preLoad)
{
	FileContents	*newfc = NULL;

	if (size > ChunkSize)
	{
		ErrMess ("File '%s' is larger than ChunkSize %ld\n", filename, ChunkSize);
	}
	
	if (size == 0)
	{
		WarnMess ("File '%s' is zero bytes long\n", filename);
	}

	//
	// see if it's the same as a previous file
	//
	{
		FileContents	*fc;

		newfc = CHK_CreateNode (sizeof (FileContents), filename, "FileContents");
        newfc->Alignment = alignment;

		if (Pack)
		{
			newfc->Data = buf;
			newfc->Size = size;
			
			fc = FindFileContentsByContent (newfc);
			
			newfc->Data = NULL;
			newfc->Size = 0;
		}

		if (Pack && (fc != NULL))
		{
			//
			// it was the same
			//
			newfc->SameAs = fc;
		}
		else
		{
			//
			// it was different
			//
			PositionNode	*pos;

			pos   = CHK_CreateNode (sizeof (PositionNode), NULL, "PositionNode");

			pos->fc = newfc;
			LST_AddTail (PosList, pos);

			newfc->Size   = size;

            size = roundUp (size, PadSize);
            
			newfc->PadSize = size;

			guessSize += size;

			if (Pack)
			{
				newfc->Data = buf;
			}
			
			// add it to hash table
			{
				HashEntry	*he;
			
				he = CHK_CallocateMemory (sizeof (HashEntry), "Hash Entry");
				he->data = newfc;
			
				HASH_AddHashEntry (FileContentsTable, he);
			}
		}

		AddFileToFileList (newfc);
	}

	return newfc;
}

/*********************************************************************
 *
 * AddUnloadedFile
 *
 * SYNOPSIS
 *		FileContents *AddUnloadedFile (char *filename, long alignment)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
FileContents* AddUnloadedFile (char *filename, long alignment)
{
	FileContents	*newfc = NULL;
	long			 size;
	uint8			*buf = NULL;
	int				 preLoad = FALSE;

	//
	// check if this file is pre-loaded
	//
	{
		PreLoadFile*	pPLF;
		
		pPLF = FindPreLoadFile (filename);
		
		if (pPLF != NULL)
		{
			// found
			preLoad = TRUE;
			
			size = pPLF->size;
			buf  = pPLF->data;
		}
		else
		{
			// not found
			int				 fh;
			fh   = CHK_ReadOpen (filename);
			size = CHK_FileLength (fh);
		
			if (size > ChunkSize)
			{
				ErrMess ("File '%s' is larger than ChunkSize %ld\n", filename, ChunkSize);
			}
			
			if (size == 0)
			{
				WarnMess ("File '%s' is zero bytes long\n", filename);
			}
		
			if (Pack)
			{
				if (Verbose)
				{
					EL_printf ("Reading %14s : size %6ld", filename, size);
				}
		
				buf = (uint8*)CHK_AllocateMemory (size, filename);
				CHK_Read (fh, buf, size);
			}
			CHK_Close (fh);
		}
	}

	//
	// see if it's the same as a previous file
	//
	newfc = AddLoadedFile (filename, alignment, buf, size, preLoad);
	if (newfc->SameAs != NULL)
	{
		if (Pack)
		{
			//
			// it was the same
			//
			if (!preLoad)
			{
				CHK_DeallocateMemory (buf, filename);
			}
			if (Verbose)
			{
				EL_printf (" : Same as %s", LST_NodeName(newfc->SameAs));
			}
		}
	}
		
	if (Pack && Verbose)
	{
		EL_printf ("\n");
	}

	return newfc;
}
// AddUnloadedFile

/*************************************************************************
                                 AddFile                                 
 *************************************************************************

   SYNOPSIS
		FileContents* AddFile (char* fname, long alignment)

   PURPOSE
  		
  
   INPUT
		fname     :
        alignment : how to align this file relative to the start of the block
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/17/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

FileContents* AddFile (char* filename, long alignment)
{
	FileContents*	 fc;
	
	// check if something is loaded with the same name
	fc = FindFileContentsByName (filename);
	if (fc)
	{
		FileContents*	newfc;
		
		newfc = CHK_CreateNode (sizeof (FileContents), filename, "FileContents");
		
		// find original file
		while (fc->SameAs != NULL)
		{
			fc = fc->SameAs;
		}
		newfc->SameAs = fc;
        
        // check that alignments agree
        if (alignment > fc->Alignment)
        {
            if (alignment % fc->Alignment)
            {
                ErrMess ("file (%s) aligned more than once and alignments are not compatible, orig = %d, new = %d\n", filename, fc->Alignment, alignment);
            }
            fc->Alignment = alignment;
        }
        else
        {
            if (fc->Alignment % alignment)
            {
                ErrMess ("file (%s) aligned more than once and alignments are not compatible, orig = %d, new = %d\n", filename, fc->Alignment, alignment);
            }
        }
		
		AddFileToFileList (newfc);

		return newfc;
	}
	
	return AddUnloadedFile (filename, alignment);
}

Level* FindLevel (char *levelName)
{
	char				 sectionName[MAX_NAME_LEN];

	sprintf (sectionName, "[%s]", levelName);

	return (Level*)LST_FindIName (LevelList, sectionName);
}
// FindLevel

bool ParseSection (Level* level, IniList* specFile, char* sectionName, ConfigLine* pCLForErrorOnly)
{
	SectionTracker		 secx;
	SectionTracker		*sec = &secx;
    bool status = TRUE;
    
	sec = FindSection (sec, specFile, sectionName);
	if (!sec)
	{
		ErrMess ("File %s, Line %d: Couldn't find Section %s\n", LocalGetConfigFilename(pCLForErrorOnly), LocalGetConfigLineNo (pCLForErrorOnly), sectionName);
		return FALSE;
	}
    
    // check for a loop
    if (INI_IsSectionUsed(sec))
    {
        INI_markSectionAsUnused(sec);
		ErrMess ("File %s, Line %d: Infinite loop in section %s\n", LocalGetConfigFilename(pCLForErrorOnly), LocalGetConfigLineNo (pCLForErrorOnly), sectionName);
        return FALSE;
    }

    INI_markSectionAsUsed(sec);
    
    // check args
    if (GetSectionArgs(sec))
    {
        char* value;
        
        NamedPairs* np = NP_SplitNamedPairs (GetSectionArgs(sec), ',', '=');
        if (!np)
        {
    		ErrMess ("File %s, Line %d: out of memory %s\n", LocalGetConfigFilename(pCLForErrorOnly), LocalGetConfigLineNo (pCLForErrorOnly), sectionName);
            return FALSE;
        }
        
        if (NP_GetValueForName(np, "align", &value))
        {
            long alignment = EL_atol(value);
            
            if (alignment)
            {
                level->alignment = alignment;
            }
        }
        
        NP_Free (np);
    }

	{
		ConfigLine		*cl;

		while ((cl = GetNextINILine (sec)) != NULL)
		{
			Part	*part;
			
			part = CHK_CreateNode (sizeof (Part), LST_NodeName(cl), LST_NodeName (cl));
			LST_AddTail (level->partsList, part);
			level->numParts++;

			if (strlen(LST_NodeName(cl)) > MAX_LINE)
			{
				ErrMess ("File %s, Line %d to long\n", GetConfigFilename(cl), GetConfigLineNo (cl));
			}
			else
			{
				#define MAX_CMD_LEN	100
				static	char	commandbuf[MAX_CMD_LEN];
				char* arg;
				char* cmd;
				char* equal;
					
				equal = strchr (LST_NodeName (cl), '=');
				if (equal && equal - LST_NodeName(cl) < MAX_CMD_LEN && equal > LST_NodeName(cl))
				{
					strncpy (commandbuf, LST_NodeName(cl), equal - LST_NodeName(cl));
					commandbuf[equal - LST_NodeName(cl)] = '\0';
					
					cmd = TrimWhiteSpace (commandbuf);
					
					arg = equal + 1;
					while (*arg && isspace(*arg)) arg++;
                
					if (!stricmp (cmd, "Data") ||
						!stricmp (cmd, "File"))
					{
						//
						// found a data part
						//
	
						char*			 fname;
                        char*            comma;
						char			 filename[EIO_MAXPATH];
                        bool             fFound = FALSE;
                        long             alignment = 1;
						
						strcpy (line, arg);
						fname = TrimWhiteSpaceAndQuotes (line);
                        
                        if ((comma = strchr (fname, ',')) != NULL)
                        {
							NamedPairs* np;
                            char* value;

                            *comma++ = '\0';
                            
                            np = NP_SplitNamedPairs (comma, ',', '=');
                            if (!np)
                            {
                				ErrMess ("File %s, Line %d OOM\n", GetConfigFilename(cl), GetConfigLineNo (cl));
                                return FALSE;
                            }
                            
                            if (NP_GetValueForName(np, "align", &value))
                            {
                                long newAlignment = EL_atol(value);
                                
                                if (newAlignment)
                                {
                                    alignment = newAlignment;
                                }
                            }
                            
                            NP_Free (np);
                        }
                        
                        if (GetConfigFilename(cl))
                        {
    						EIO_fnmerge (filename, EIO_Path(GetConfigFilename(cl)), fname, NULL);
                            fFound = EIO_FileExists (filename);
                        }
                        if (!fFound)
                        {
    						EIO_fnmerge (filename, inputPath, fname, NULL);
                            if (!EIO_FileExists (filename))
                            {
                                ErrMess ("File %s, Line %d: could not open file (%s)\n", GetConfigFilename(cl), GetConfigLineNo (cl), fname);
                            }
                        }
                        
						part->type   = PART_DATA;
						part->fc     = AddFile (filename, alignment);
						part->size   = sizeof (void *);
					}
					else if (!stricmp (cmd, "load"))
					{
						//
						// found a runtime load part
						//
						
						// hack together new section for this file
						char*			fname;
						char			secname[EIO_MAXPATH * 2];
						char			secnameb[EIO_MAXPATH * 2];
						
						strcpy (line, arg);
						fname = TrimWhiteSpaceAndQuotes (line);
						
						sprintf (secname, "___runtime_%s_file___", fname);
						sprintf (secnameb, "[___runtime_%s_file___]", fname);
						
						part->level = FindLevel (secname);
						if (!part->level)
						{
							// add a 
							Section*	pLocalSec;
							char		localline[1024];
							
							pLocalSec = AddINISection (specFile, secnameb);
							if (!pLocalSec) { FailMess ("OOM: sec for runtime file"); }
							
							sprintf (localline, "string=%s", fname);
							if (!AddINILine (pLocalSec, "long=0",  1, NULL)) { FailMess ("OOM: line 1 for runtime file"); }
							if (!AddINILine (pLocalSec, localline, 2, NULL)) { FailMess ("OOM: line 2 for runtime file"); }
							if (!AddINILine (pLocalSec, "byte=0",  3, NULL)) { FailMess ("OOM: line 3 for runtime file"); }
							
							part->level = ParseLevel (specFile, secname, cl);
						}
	
						part->type   = PART_RUNTIMEFILE;
						part->size   = sizeof (void *);
					}
					else if (!stricmp (cmd, "Level") ||
							 !stricmp (cmd, "Pntr"))
					{
						//
						// found a level part
						//
						part->type   = PART_LEVEL;
						part->size   = sizeof (void *);
	
						part->level = FindLevel (arg);
						if (!part->level)
						{
							part->level = ParseLevel (specFile, arg, cl);
						}
					}
					else if (!stricmp (cmd, "align"))
					{
						//
						// found align part
						//
                        long alignment = EL_atol(arg);
                        long pad       = 0;
                        
                        if (alignment)
                        {
                            long currentOffset = level->size % alignment;
                            if (currentOffset)
                            {
                                pad = alignment - currentOffset;
                            }
                        }
						part->type   = PART_ALIGN;
						part->size   = pad;
					}
					else if (!stricmp (cmd, "pad"))
					{
						//
						// found pad part
						//
                        long padding = EL_atol(arg);
                        
						part->type   = PART_PAD;
						part->size   = padding;
					}
					else if (!stricmp (cmd, "Long") ||
                             !stricmp (cmd, "int32") ||
                             !stricmp (cmd, "uint32"))
					{
						int	 numargs;
	
						strcpy (line, arg);
						numargs = argify (line, MAX_ARGS, args);
	
						//
						// found a Long part
						//
						part->type   = PART_LONG;
						part->string = arg;
						part->size   = sizeof (uint32) * numargs;
					}
					else if (!stricmp (cmd, "Word") ||
                             !stricmp (cmd, "Short") ||
                             !stricmp (cmd, "int16") ||
                             !stricmp (cmd, "uint16"))
					{
						//
						// found a Word part
						//
						int	 numargs;
	
						strcpy (line, arg);
						numargs = argify (line, MAX_ARGS, args);
	
						part->type   = PART_WORD;
						part->string = arg;
						part->size   = sizeof (uint16) * numargs;
					}
					else if (!stricmp (cmd, "Byte") ||
                             !stricmp (cmd, "char") ||
                             !stricmp (cmd, "uint8") ||
                             !stricmp (cmd, "int8"))
					{
						//
						// found a Byte part
						//
						int	 numargs;
	
						strcpy (line, arg);
						numargs = argify (line, MAX_ARGS, args);
	
						part->type   = PART_BYTE;
						part->string = arg;
						part->size   = sizeof (uint8) * numargs;
					}
					else if (!stricmp (cmd, "Float"))
					{
						//
						// found a Float part
						//
						int	 numargs;
	
						strcpy (line, arg);
						numargs = argify (line, MAX_ARGS, args);
	
						part->type   = PART_FLOAT;
						part->string = arg;
						part->size   = sizeof (float) * numargs;
					}
					else if (!stricmp (cmd, "String"))
					{
						//
						// found a String part
						//
						part->type   = PART_STRING;
						part->string = arg;
						part->size   = strlen (arg);
					}
					else if (!stricmp (cmd, "binc"))
					{
						//
						// found a Binary include part
						//
						char			 filename[EIO_MAXPATH];
						char*			 fname;
						int				 fh;
						long			 size;
                        bool             fFound = FALSE;
						
						strcpy (line, arg);
						fname = TrimWhiteSpaceAndQuotes (line);

                        if (GetConfigFilename(cl))
                        {
    						EIO_fnmerge (filename, EIO_Path(GetConfigFilename(cl)), fname, NULL);
                            fFound = EIO_FileExists (filename);
                        }
                        if (!fFound)
                        {
    						EIO_fnmerge (filename, inputPath, fname, NULL);
                            if (!EIO_FileExists (filename))
                            {
                                ErrMess ("File %s, Line %d: could not open file (%s)\n", GetConfigFilename(cl), GetConfigLineNo (cl), fname);
                            }
                        }
						
						fh   = CHK_ReadOpen (filename);
						size = CHK_FileLength (fh);
						CHK_Close (fh);
	
						part->type   = PART_BINC;
						part->string = CHK_dupstr(filename);
						part->size   = size;
					}
					else if (!stricmp (cmd, "Path"))
					{
						//
						// found a Path part
						//
						part->type   = PART_PATH;
						part->string = arg;
					//	level->size += 0;
						inputPath = part->string;
					}
                    else if (!stricmp (cmd, "Insert"))
                    {
                    	char subSectionName[MAX_NAME_LEN];
                        
                    	sprintf (subSectionName, "[%s]", arg);
    
                        status = ParseSection (level, specFile, subSectionName, cl);
                    }
					else
					{
						ErrMess ("File %s, Line %d: Unknown specFile '%s'\n", GetConfigFilename(cl), GetConfigLineNo (cl), LST_NodeName (cl));
					}
					level->size += part->size;
				}
			}
		}
	}

    INI_markSectionAsUnused(sec);
    
    return status;
}

/*********************************************************************
 *
 * ParseLevel
 *
 * SYNOPSIS
 *		Level *ParseLevel (IniList *specFile, char *levelName)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
Level* ParseLevel (IniList *specFile, char *levelName, ConfigLine* pCL)
{
	Level				*level;
	char				 sectionName[MAX_NAME_LEN];

	if (Verbose)	EL_printf ("Parsing Level [%s]\n", levelName);

	sprintf (sectionName, "[%s]", levelName);

	level = CHK_CreateNode (sizeof (Level), sectionName, sectionName);

	level->partsList = &level->partsListX;
	LST_InitList (level->partsList);
	LST_AddTail (LevelList, level);
    
    if (!ParseSection (level, specFile, sectionName, pCL))
    {
        // todo: this is sloppy.  I don't cleanup here. but since I got an error
        //       I'm not going to finish anyway so ...
        
        return NULL;
    }

	return level;
}
// ParseLevel

/*************************************************************************
                               ReadPreLoad                               
 *************************************************************************

   SYNOPSIS
		int ReadPreLoad (char *filename)

   PURPOSE
  		
  
   INPUT
		filename :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		01/16/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int ReadPreLoad (char *filename)
{
	int			done = FALSE;
	int			fh;
	long		len;
	uint8*		data;
	LST_LIST	sameList;
	
	LST_InitList (&sameList);
	
	// read entire file into memory	
	fh   = CHK_ReadOpen (filename);
	len  = CHK_FileLength (fh);
	data = (uint8*)CHK_AllocateMemory (len, filename);
	
	CHK_Read (fh, data, len);
	CHK_Close (fh);
	
	// check version
	{
		long	version;
		
		version = *((long*)data);
		data   += 4;
		version = LSBFToNative32Bit(version);
		
		if (version != PRELOAD_VERSION)
		{
			FailMess ("Bad version for pre load file %s\n", filename);
		}
	}
	
	// add files to PreLoadFilelist
	while (!done)
	{
		PreLoadFile*	pPLF;
		char*	name;
		long	size;
		long	type;
		
		type = *((long*)data);
		data += 4;
		type  = LSBFToNative32Bit(type);
		
		switch (type)
		{
		case 0:	// end of file
			done = TRUE;
			break;
		case 1: // file data
			{
				size  = *((long*)data);
				data += 4;
				name  = data;
				data += strlen (name) + 1;
				size  = LSBFToNative32Bit(size);
				
				if (Verbose)
				{
					EL_printf ("Preloading %14s : size %6ld\n", name, size);
				}
				
				pPLF = (PreLoadFile*)CHK_CreateNode (sizeof (PreLoadFile), name, "PreLoadFile");
				
				pPLF->size = size;
				pPLF->data = data;
				
				data += size;
				
				LST_AddTail (PreLoadFileList, pPLF);
			
				{
					HashEntry	*he;
				
					he = CHK_CallocateMemory (sizeof (HashEntry), "Hash Entry");
					he->data = pPLF;
				
					HASH_AddHashEntry (PreLoadTable, he);
				}
			}
			break;
		case 2:	// same as
			{
				char*	sameName;
				
				name      = data;
				data     += strlen (name) + 1;
				sameName  = data;
				data     += strlen (sameName) + 1;
				
				if (Verbose)
				{
					EL_printf ("Preloading %14s : size %6ld : same as %14s\n", name, size, sameName);
				}
				
				pPLF = (PreLoadFile*)CHK_CreateNode (sizeof (PreLoadFile), name, "PreLoadFile");
				
				pPLF->sameAs = sameName;
				
				LST_AddTail (&sameList, pPLF);
			}
			break;
		default:
			FailMess ("unknown type %d in preload file %s\n", type, filename);
			break;
		}
	}
	
	// add same as ones to list now that we're sure the ones that they're the same
	// as ones that are loaded
	{
		PreLoadFile* pPLF;
		
		while ((pPLF = (PreLoadFile*)LST_RemHead (&sameList)) != NULL)
		{
			PreLoadFile*	samePLF;
			HashEntry*		he;
			
			samePLF = FindPreLoadFile (pPLF->sameAs);
			
			ENSURE_F (samePLF == NULL, ("Impossible! Didn't find a match\n"));
			
			pPLF->data       = samePLF->data;
			pPLF->size       = samePLF->size;
			pPLF->sameAsFile = samePLF;
			
			LST_AddTail (PreLoadFileList, pPLF);
			
			he = CHK_CallocateMemory (sizeof (HashEntry), "Hash Entry");
			he->data = pPLF;
			
			HASH_AddHashEntry (PreLoadTable, he);
		}
	}
	
	RETURN TRUE;
}

/*************************************************************************
                              WritePreLoad                               
 *************************************************************************

   SYNOPSIS
		void WritePreLoad (char* filename)

   PURPOSE
  		
  
   INPUT
		filename :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		01/16/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void WritePreLoad (char* filename, char* keyname)
{
	long zero = NativeToLSBF32Bit(0);
	long one  = NativeToLSBF32Bit(1);
	long two  = NativeToLSBF32Bit(2);

	if (filename)
	{
		FileContents*	fc;
		int	fh;
		
		fh = CHK_WriteOpen (filename);
		
		{
			long	version = NativeToLSBF32Bit(PRELOAD_VERSION);
			
			CHK_Write (fh, &version, sizeof (version));
		}
	
		fc = (FileContents*)LST_Head (FileList);
		while (!LST_EndOfList (fc))
		{
			if (!fc->bFromPreLoad)
			{
				FileContents*	dataFc;
				
				dataFc = fc;
				while (dataFc->SameAs != NULL)
				{
					dataFc = dataFc->SameAs;
				}
				
				if (dataFc->Data)
				{
					if (fc->SameAs != NULL)
					{
						long	size;
						
						size = fc->Size;
						size = NativeToLSBF32Bit(size);
						
						CHK_Write (fh, &two, sizeof (two));
						CHK_Write (fh, LST_NodeName(fc), strlen (LST_NodeName(fc)) + 1);
						CHK_Write (fh, LST_NodeName(dataFc), strlen (LST_NodeName(dataFc)) + 1);
					}
					else
					{
						long	size;
						
						size = fc->Size;
						size = NativeToLSBF32Bit(size);
						
						CHK_Write (fh, &one, sizeof (one));
						CHK_Write (fh, &size, sizeof (size));
						CHK_Write (fh, LST_NodeName(fc), strlen (LST_NodeName(fc)) + 1);
						if (fc->Size)
						{
							CHK_Write (fh, fc->Data, fc->Size);
					    }
					}
				}
			}
			fc = (FileContents*)LST_Next (fc);
		}
	
		CHK_Write (fh, &zero, sizeof (zero));
		
		CHK_Close (fh);
	}
	
	if (keyname)
	{
		LST_LIST		SortList;
		FileContents*	fc;
		
		LST_InitList (&SortList);
		
		fc = (FileContents*)LST_Head (FileList);
		while (!LST_EndOfList (fc))
		{
			LST_NODE*	nd;
			
			nd = LST_Head (&SortList);
			while (!LST_EndOfList(nd))
			{
				int	r;

				r = stricmp (LST_NodeName(fc), LST_NodeName(nd));
				if (r < 0)
				{
					LST_InsertBefore (nd, CHK_CreateNode(sizeof(LST_NODE), LST_NodeName(fc), "SortNode"));
					break;
				}
				if (r == 0)
				{
					break;
				}
				nd = LST_Next (nd);
			}
			
			if (LST_EndOfList(nd))
			{
				LST_InsertBefore (nd, CHK_CreateNode(sizeof(LST_NODE), LST_NodeName(fc), "SortNode"));
			}
			
			fc = (FileContents*)LST_Next(fc);
		}
		
		{
			LST_NODE*	nd;
			FILE*		fp;

			fp = CHK_fopen (keyname, "w");
		
			nd = LST_Head (&SortList);
			while (!LST_EndOfList(nd))
			{
				strlwr (LST_NodeName(nd));
				CHK_fprintf (fp, "%s\n", LST_NodeName(nd));

				nd = LST_Next (nd);
			}
		
			CHK_fclose(fp);
		}
	}
}

/*************************************************************************
                              CopyIntoFile                               
 *************************************************************************

   SYNOPSIS
		int CopyIntoFile (int outfh, const char*infilename)

   PURPOSE
  		copy the bytes from one file into a currently opened file
  
   INPUT
		outfh       : file handle for output file
		infilename  :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		05/09/02 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int CopyIntoFile (int outfh, const char*infilename)
{
	static	uint8* buf = NULL;
	
	int		infh;
	long	bytes;
	
	if (!buf)
	{
		buf = CHK_AllocateMemory (BUF_SIZE, "copy buffer");
	}

	infh = CHK_ReadOpen (infilename);
	bytes = CHK_FileLength (infh);
	while (bytes)
	{
		long	len;
	
		len = min (bytes, BUF_SIZE);
		if (len != CHK_Read (infh, buf, len))
		{
			FailMess ("Trouble reading %s\n", infilename);
		}
		if (len != MK_Write (outfh, buf, len))
		{
			return FALSE;
		}
	
		bytes -= len;
	}

	CHK_Close (infh);
	
	return TRUE;
}

/*************************************************************************
                             WriteOutFixups                              
 *************************************************************************

   SYNOPSIS
		WriteOutFixups (int fh)

   PURPOSE
  		Write out the fixups and an end zero
  
   INPUT
		fh :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		05/17/02 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void WriteOutFixups (int fh)
{
	Level	*level;

	level = (Level*)LST_Head (LevelList);
	while (!LST_EndOfList (level))
	{
		Part	*part;
		long	 position;

		position = FilePosition (level->fc);

		part = (Part*)LST_Head (level->partsList);
		while (!LST_EndOfList (part))
		{
			switch (part->type)
			{
			case PART_DATA:
			case PART_LEVEL:
				WritePosition (fh, position, FALSE, LST_NodeName(part));
				break;
			case PART_RUNTIMEFILE:
				WritePosition (fh, position, TRUE, LST_NodeName(part));
				break;
			default:
				break;
			}
			position += part->size;
			part = (Part*)LST_Next (part);
		}
		level = (Level*)LST_Next (level);
	}
	{
		uint32	zero = 0;

		MK_Write (fh, &zero, sizeof (uint32));
	}
}

void WritePadding (int fh, long padding)
{
    while (padding > 0)
    {
        long	part;

        part = min (padding, sizeof(ZeroData));
        MK_Write (fh, ZeroData, part);
        padding -= part;
    }
}

/******************************** TEMPLATE ********************************/

#define ARG_OUTFILE		(newargs[ 0])
#define ARG_SPECFILE	(newargs[ 1])
#define ARG_VERBOSE		(newargs[ 2])
#define	ARG_DONTOUT		(newargs[ 3]) 
#define	ARG_DUPERR		(newargs[ 4])
#define ARG_PACK		(newargs[ 5]) 
#define ARG_BIGENDIAN	(newargs[ 6]) 
#define ARG_BYTES		(newargs[ 7]) 
#define ARG_CHUNK		(newargs[ 8]) 
#define	ARG_HARDWARE	(newargs[ 9]) 
#define ARG_FIXUPS		(newargs[10]) 
#define	ARG_PADEND		(newargs[11]) 
#define ARG_TOPSECTION	(newargs[12]) 
#define	ARG_WRITELOAD	(newargs[13]) 
#define	ARG_KEYFILE		(newargs[14]) 
#define ARG_DONTSORT	(newargs[15])
#define	ARG_READLOAD	(newargs[16])
#define	ARG_INCPATH		(newargs[17])

char Usage[] = "Usage: MKLOADOB OUTFILE SPECFILES [switches...]\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",		"\tOUTFILE             = Output filename\n", },
{STANDARD_ARG|REQUIRED_ARG|MULTI_ARG,	"SPECFILES",	"\tSPECFILES           = File(s) of levels and data\n", },
{SWITCH_ARG,							"-VERBOSE",		"\t-VERBOSE            = Verbose (show errors)\n", },
{SWITCH_ARG,							"-NOOUTPUT",	"\t-NOOUTPUT           = Don't write output file\n", },
{SWITCH_ARG,							"-DUPERR",		"\t-DUPERR             = Duplicate sections cause error (default, ignore duplicates)\n", },
{SWITCH_ARG,							"-NOPACK",		"\t-NOPACK             = Don't Pack\n", },
{SWITCH_ARG,							"-BIGENDIAN",	"\t-BIGENDIAN          = Big Endian\n", },
{KEYWORD_ARG,							"-PADSIZE",		"\t-PADSIZE <bytes>    = Padsize Size (Def. 4)\n", },
{KEYWORD_ARG,							"-CHUNKSIZE",	"\t-CHUNKSIZE <bytes>  = Chunk Size (Def. 2048)\n", },
{KEYWORD_ARG,							"-SECTORSIZE",	"\t-SECTORSIZE <bytes> = Hardware Sector Size (Def. 1)\n", },
{KEYWORD_ARG,							"-FIXUPMODE",	"\t-FIXUPMODE <mode>   = Fixup mode (0 = none (def), 1 = at beginning, 2 = at end\n", },
{SWITCH_ARG,							"-NOPAD",		"\t-NOPAD              = Don't pad end of file to Chunk size\n", },
{KEYWORD_ARG,							"-TOP",			"\t-TOP <top>          = Top Section (Def. 'START')\n", },
{KEYWORD_ARG,							"-WRITEPRE",	"\t-WRITEPRE <prefile> = Write Preload file\n", },
{KEYWORD_ARG,							"-WRITEKEY",	"\t-WRITEKEY <prekey>  = Write Preload key file\n", },
{SWITCH_ARG,							"-NOSORT",		"\t-NOSORT             = Do NOT sort binary files\n", },	// the point of this is supposed to be that if not sorted, "file=" files will generally get loaded in memory consecutively as they are found in the spec file.  Of course they will be no where near the non-file data
{KEYWORD_ARG|MULTI_ARG,					"-READPRE",		"\t-READPRE <prefile>  = Read prefile\n", },
{KEYWORD_ARG|MULTI_ARG,					"-INCLUDE",		"\t-INCLUDE <incpath>  = Add an include path\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char	**newargs;
	long	 blockTableSize;
	long	 fixupBeforeTableSize; // includes top pointer
	long	 fixupAfterTableSize;
	long	 totalSize;

	EL_printf ("MKLOADOB Copyright (c) 1997-2002 Echidna\n");

	LST_InitList (PreLoadFileList);
	LST_InitList (FileList);
	LST_InitList (PosList);
	LST_InitList (SortedList);
	LST_InitList (LevelList);
	
	PreLoadTable = HASH_CreateHashTable (
							LST_NodeHashFunc,
							LST_NodeHashCmpFunc,
							4096);
								
	FileTable = HASH_CreateHashTable (
							LST_NodeHashFunc,
							LST_NodeHashCmpFunc,
							4096);
	
	FileContentsTable = HASH_CreateHashTable (
							FileContentsHashFunc,
							FileContentsHashCmpFunc,
							4096);

	SetINIMergeSections(FALSE);
	SetINICaseSensitive(FALSE);
    SetINIUndefEnvVarIsError(TRUE);
    SetINIUseMacroLanguage(TRUE);
    SetINIStripCPlusPlusComments(TRUE);
    SetINIParseArgsInSection(TRUE);
    SetINIChangeCurrentDir(TRUE);

	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	}
	else
	{

		Verbose      =  SWITCH_VALUE(ARG_VERBOSE);
		Pack         = !SWITCH_VALUE(ARG_PACK);
		LittleEndian = !SWITCH_VALUE(ARG_BIGENDIAN);
		PadEnd       = !SWITCH_VALUE(ARG_PADEND);
		DontOut      =  SWITCH_VALUE(ARG_DONTOUT);
		fDontSort    =  SWITCH_VALUE(ARG_DONTSORT);
        fDupErr      =  SWITCH_VALUE(ARG_DUPERR);
        
        SetINIErrorOnDuplicateSection(fDupErr);

		if (ARG_TOPSECTION)
		{
			topSection = ARG_TOPSECTION;
		}

		if (ARG_FIXUPS)		UseFixupsMode      = EL_atol (ARG_FIXUPS);
		if (ARG_BYTES)		PadSize            = EL_atol (ARG_BYTES);
		if (ARG_CHUNK)		ChunkSize          = EL_atol (ARG_CHUNK);
		if (ARG_HARDWARE)	HardwareSectorSize = EL_atol (ARG_HARDWARE);
		
		if (PadSize <= 3)
		{
			FailMess ("Padsize must be > 3\n");
		}
		if (PadSize & 0x3)
		{
			FailMess ("Padsize must not use lower 2 bits (4, 8, 16, 32, 64 are okay, 7 is not!)\n");
		}
		
		// figure out the offset and block bits based on the chunk and pad size
		{
			long temp;
			long numOffsetBits = 0;
			long numBlockBits;
			
			// find out how many bits are unused in a position because of the pad size;
            #if 0
			temp = PadSize;
			while (!(temp & 0x01))
            {
				temp >>= 1;
                PositionOffsetShift++;
			}
            PositionOffsetShift -= 2; // flags
            #else

            /*            
              NOTE: The reason we can't currently calculate a position offset
                     is because fixups point to non offset areas.  In other
                     words even though all pointers in the data will be on
                     padsize boundries, the fixup pointers point to like
                     individual fields inside a structure/level that need
                     fixing.
                     
                    I'm not sure how I could fix this.  I can't put the bits
                    at the top of the offset because some platforms start
                    the ram address at > 0x4000000 or > 0x8000000.
                    
                    the only thing I can think of is making the fixups a
                    completely different format
            */
            PositionOffsetShift = 0;
            #endif
			
			// find out how many bits are needed for an offset
			temp = (ChunkSize - 1);
			while (temp)
			{
				temp >>= 1;
				numOffsetBits++;
			}

			// block bits is			
			// 32 bits - the number of bits used by the offset
			// minus the number not used because of the pad
			numBlockBits = 32 - (numOffsetBits - PositionOffsetShift);
			MaxBlocks    = 1L << numBlockBits;
			
			PositionBlockShift = (numOffsetBits - PositionOffsetShift);
		}
		
		//
		// add include paths
		//
		{
			char			**files;
			
			files = MULTI_ARGLIST (ARG_INCPATH);
			if (files)
			{
				while (*files)
				{
					EIO_AddIncludePath (EIO_INCPATH_INIS, *files);

					files++;
				}

			}
		}
		
		//
		// load pre-load stuff
		//
		{
			char			**files;
			
			files = MULTI_ARGLIST (ARG_READLOAD);
			if (files)
			{
				while (*files)
				{
					if (Verbose)	EL_printf ("=====Reading Preload File %s=====\n", *files);

					ReadPreLoad (*files);

					files++;
				}

			}
		}

		//---------------------------------------------
		// Parse SPECFILE and create list of data files to load and levels
		//---------------------------------------------
		{
			IniList			*specFile = NULL;
			char			**files;

			files = MULTI_ARGLIST (ARG_SPECFILE);

			while (*files)
			{
				if (Verbose)	EL_printf ("=====Reading Spec File %s=====\n", *files);

				specFile = AppendINI (specFile, *files);
				if (!specFile)
				{
					FailMess ("Reading specfile '%s'\n", *files);
				}
				files++;
			}
			ParseLevel (specFile, topSection, NULL);
		}

		//---------------------------------------------
		// add level 'parts'
		//---------------------------------------------
		if (!ErrorCount)
		{
			Level	*level;

			level = (Level*)LST_Head (LevelList);

			if (Verbose)	EL_printf ("=====Adding Level Parts=====\n");

			while (!LST_EndOfList (level))
			{

				long size;
				
				size = level->size;
				
				if (Verbose)	EL_printf ("=====Adding Level %s=====\n", LST_NodeName (level));

				if (size > ChunkSize)
				{
					ErrMess ("File '%s' is larger than ChunkSize %ld\n", LST_NodeName(level), ChunkSize);
				}
				
				if (size == 0)
				{
					WarnMess ("Level '%s' is zero bytes long\n", LST_NodeName(level));
				}

				{
					FileContents	*newfc;
					PositionNode	*pos;

					newfc = CHK_CreateNode (sizeof (FileContents), LST_NodeName(level), "FileContents");
					newfc->Level     = level;
                    newfc->Alignment = level->alignment;
					level->fc        = newfc;

					pos   = CHK_CreateNode (sizeof (PositionNode), NULL, "PositionNode");

					pos->fc = newfc;
					LST_AddTail (PosList, pos);

					newfc->Size   = size;

                    size = roundUp (size, PadSize);

					newfc->PadSize = size;

					guessSize += size;

					AddFileToFileList (newfc);
				}

				level = (Level*)LST_Next (level);
			}
		}

		if (ARG_WRITELOAD || ARG_KEYFILE)
		{
			WritePreLoad (ARG_WRITELOAD, ARG_KEYFILE);
		}

		if (DontOut)
		{
			goto done;
		}

/************************* Sort and Position files ************************/

		//---------------------------------------------
		// figure out space for fixups
		//---------------------------------------------
		if (UseFixupsMode)
		{
			Level	*level;

			level = (Level*)LST_Head (LevelList);
			while (!LST_EndOfList (level))
			{
				Part	*part;

				part = (Part*)LST_Head (level->partsList);
				while (!LST_EndOfList (part))
				{
					switch (part->type)
					{
					case PART_DATA:
					case PART_LEVEL:
					case PART_RUNTIMEFILE:
						TotalFixups++;
						break;
					default:
						break;
					}
					part = (Part*)LST_Next (part);
				}
				level = (Level*)LST_Next (level);
			}
		}

		if (!ErrorCount)
		{
			long	count = 0;
			long	curAddress;

			switch (UseFixupsMode)
			{
			case 1: // put table at beginning (why, I don't know)
				// fixup table at beginning
				//   (*) start pointer
				//   (*) fixup table
				//   (*) fixup table endmarker
				fixupBeforeTableSize = (TotalFixups + 1) * sizeof (uint32) + sizeof (uint32);
				// fixup table at end
				//   (*) nothing
				fixupAfterTableSize  = 0;
				break;
			case 2: // *** put table at end so you can free it
				// fixup table at beginning
				//   (*) start pointer
				//   (*) fixup table pointer
				fixupBeforeTableSize = 2 * sizeof (uint32);
				// fixup table at end
				//   (*) fixup table
				//   (*) fixup table endmarker
				fixupAfterTableSize  = (TotalFixups + 1) * sizeof (uint32);
				break;
			default: // *** no fixup table
				// fixup table at beginning
				//   (*) start pointer
				//   (*) fixup table pointer
				fixupBeforeTableSize = sizeof (uint32);
				// fixup table at end
				//   (*) nothing
				fixupAfterTableSize  = 0;
				break;
			}

			//
			// try until it works
			//
			for (;;)
			{
				if (Verbose)
				{
					EL_printf ("------------------------\n");
					EL_printf ("starting guessSize = %ld\n", guessSize);
				}

				blockTableSize  = (((guessSize + ChunkSize - 1) / ChunkSize) + 1) * sizeof (uint32);
				curAddress      = blockTableSize + fixupBeforeTableSize;
                curAddress      = roundUp (curAddress, PadSize);
				guessSize		= guessSize + curAddress;

				if (Verbose)
				{
					EL_printf ("new guessSize  = %ld\n", guessSize);
					EL_printf ("blockTableSize = %ld\n", blockTableSize);
				}

				//
				// sort files largest to smallest
				//
				if (fDontSort)
				{
					LST_MoveListBeforeNode (PosList, LST_Head (SortedList));
				}
				else
				{
					PositionNode	*pos;

					while ((pos = (PositionNode*)LST_RemHead (PosList)) != NULL)
					{
						PositionNode	*old;

						old = (PositionNode*)LST_Head (SortedList);
						for (;;)
						{
							if (LST_EndOfList (old))
							{
								LST_InsertBefore (old, pos);
								break;
							}
							if (pos->fc->Size > old->fc->Size)
							{
								LST_InsertBefore (old, pos);
								break;
							}
							old = (PositionNode*)LST_Next (old);
						}
					}
				}

				//
				// add files to list
				//
				while (!LST_IsEmpty (SortedList))
				{
					PositionNode	*pos;
					FileContents	*fc;

					pos = (PositionNode*)LST_Head (SortedList);
					while (!LST_EndOfList (pos))
					{
						long			 endAddress;

						fc = pos->fc;
                        
						endAddress = curAddress + fc->PadSize;
                        
                        // align this section if we need to
                        if (fc->Alignment && curAddress % fc->Alignment)
                        {
                            endAddress = roundUp(curAddress, fc->Alignment) - curAddress;
                        }

						if ((curAddress + ChunkSize - 1) / ChunkSize == (endAddress + ChunkSize - 1) / ChunkSize)
						{
							break;
						}

						pos = (PositionNode*)LST_Next (pos);
					}

					if (LST_EndOfList (pos))
					{
						//
						// No file that fits was found so look move to next
						// boundry and just use first file
						//
                        curAddress = roundUp (curAddress, ChunkSize);

						pos = (PositionNode*)LST_Head (SortedList);
						fc  = pos->fc;
					}

					LST_Remove (pos);
					LST_AddTail (PosList, pos);

                    if (fc->Alignment)
                    {
                        curAddress = roundUp (curAddress, fc->Alignment);
                    }
					fc->Offset  = curAddress;
					curAddress += fc->PadSize;
				}

				//
				// all the files have been added
				// did the sector count match?
				//

				if (((uint32)curAddress + ChunkSize - 1) / ChunkSize <=
					(blockTableSize - 1) / sizeof (uint32))
				{
					break;
				}

				count++;

				if (Verbose)
				{
					EL_printf ("Sorting %d\n", count);
				}

				guessSize = curAddress;
			}

			totalSize = curAddress;
		}

/******************************* Write Files ******************************/

		if (!ErrorCount)
		{
			int				 fh;
			long			 headerTableSize;
			long			 fixupFileSeekPosition;
			long			 slack = 0;

			fh = CHK_WriteOpen (ARG_OUTFILE);

			headerTableSize = fixupBeforeTableSize + blockTableSize;

			if (headerTableSize > ChunkSize)
			{
				FailMess ("Too many files or size to many sectors\n");
			}

			if (Verbose)
			{
				EL_printf ("fixup before Table Size = %ld\n", fixupBeforeTableSize);
				EL_printf ("block Table Size        = %ld\n", blockTableSize);
				EL_printf ("header Table Size       = %ld\n", headerTableSize);
				EL_printf ("fixup after Table Size  = %ld\n", fixupAfterTableSize);

				EL_printf ("Writing %ld bytes to file %s\n", PadEnd ? (((totalSize + ChunkSize - 1) / ChunkSize) * ChunkSize) : totalSize, ARG_OUTFILE);
			}

			//---------------------------------------------
			// write block flags
			//---------------------------------------------
			{
				long	blocks;

				blocks = (totalSize + ChunkSize - 1) / ChunkSize;
				blocksMarked = blocks;
				
				while (blocks)
				{
					uint32	flag = 0xFFFFFFFFUL;

					MK_Write (fh, &flag, sizeof (uint32));

					blocks--;
				}

				{
					uint32	zero = 0;

					MK_Write (fh, &zero, sizeof (uint32));
				}
			}

			//---------------------------------------------
			// write pointer to top level
			//---------------------------------------------
			{
				Level	*level;

				level = (Level*)LST_Head (LevelList);

				WritePosition (fh, FilePosition(level->fc), FALSE, LST_NodeName(level));
			}

			//---------------------------------------------
			// write fixups
			//---------------------------------------------
			if (UseFixupsMode == 1)
			{
				WriteOutFixups (fh);
			}
			else if (UseFixupsMode == 2)
			{
				uint32	zero = 0;

				// we need to write the offset to the fixups,
				// they will be at the end of the file AND they 
				// are NOT compatible with the block system
				// so the block size must be set high so everything
				// fits in one block

				fixupFileSeekPosition = CHK_Seek (fh, 0, EIO_SEEK_CURRENT);				
				MK_Write (fh, &zero, sizeof (uint32));
			}

			#if 0
			//---------------------------------------------
			// write file offsets
			//---------------------------------------------
			{
				FileContents	*fc;

				fc = (FileContents*)LST_Head (FileList);
				while (!LST_EndOfList (fc))
				{
					WritePosition (fh, FilePosition(fc), LST_NodeName(fc));
			
					fc = (FileContents*)LST_Next (fc);
				}

				{
					uint32	zero = 0;

					MK_Write (fh, &zero, sizeof (uint32));
				}
			}
			#endif
            
            // pad header to padsize
            if (BytesWritten % PadSize)
            {
                WritePadding (fh, PadSize - BytesWritten % PadSize);
            }

			//---------------------------------------------
			// write files
			//---------------------------------------------
			{
				PositionNode	*pos;

				pos = (PositionNode*)LST_Head (PosList);
				while (!LST_EndOfList (pos))
				{
					FileContents	*fc;
					long			 offset;
                    
					fc = pos->fc;
                    
                    // align this section if we need to
                    if (fc->Alignment && BytesWritten % fc->Alignment)
                    {
                        WritePadding (fh, fc->Alignment - BytesWritten % fc->Alignment);
                    }

					offset = FilePosition(fc);

					if (offset != BytesWritten)
					{
						ErrMess ("Offset $%08lx : BytesWritten $%08lx\n", offset, BytesWritten);
					}

					if (fc->Level)
					{
						if (Verbose)
						{
                        	long	lc_block;
                        	long	lc_offset;
                        
                        	lc_block    = BytesWritten / ChunkSize;
                        	lc_offset   = BytesWritten % ChunkSize;
                            
                    		EL_printf ("Block $%04lx : Offset $%04lx : ", lc_block, lc_offset);
							EL_printf ("Writing %7ld bytes from %s\n", fc->Size, LST_NodeName (fc));
						}

						{
							Part	*part;

							part = (Part*)LST_Head (fc->Level->partsList);
							while (!LST_EndOfList (part))
							{
								switch (part->type)
								{
								case PART_DATA:
									WritePosition (fh, FilePosition(part->fc), FALSE, LST_NodeName(part));
									break;
								case PART_LEVEL:
									WritePosition (fh, FilePosition(part->level->fc), FALSE, LST_NodeName(part));
									break;
								case PART_RUNTIMEFILE:
									WritePosition (fh, FilePosition(part->level->fc), TRUE, LST_NodeName(part));
									break;
								case PART_LONG:
									{
										uint32	 vl;
										int	 numargs;
										int	 i;

										strcpy (line, part->string);
										numargs = argify (line, MAX_ARGS, args);

										for (i = 0; i < numargs; i++)
										{
											vl = EL_atol (args[i]);

											if (LittleEndian)
											{
												MakeLilLong(vl);
											}
											else
											{
												MakeBigLong(vl);
											}

											MK_Write (fh, &vl, sizeof (uint32));
										}
									}
									break;
								case PART_WORD:
									{
										uint16	 vw;
										int	 numargs;
										int	 i;

										strcpy (line, part->string);
										numargs = argify (line, MAX_ARGS, args);

										for (i = 0; i < numargs; i++)
										{
											vw = (uint16)EL_atol (args[i]);

											if (LittleEndian)
											{
												MakeLilWord(vw);
											}
											else
											{
												MakeBigWord(vw);
											}

											MK_Write (fh, &vw, sizeof (uint16));
										}
									}
									break;
								case PART_BYTE:
									{
										uint8	 vb;
										int	 numargs;
										int	 i;

										strcpy (line, part->string);
										numargs = argify (line, MAX_ARGS, args);

										for (i = 0; i < numargs; i++)
										{
											vb = (uint8)EL_atol (args[i]);

											MK_Write (fh, &vb, sizeof (uint8));
										}
									}
									break;
								case PART_FLOAT:
									{
										float	 vf;
										int	 numargs;
										int	 i;

										strcpy (line, part->string);
										numargs = argify (line, MAX_ARGS, args);

										for (i = 0; i < numargs; i++)
										{
											vf = (float)atof (args[i]);

											if (LittleEndian)
											{
												#if __LITTLEENDIAN__
													MK_Write (fh, &vf, sizeof (float));
												#else
													{
														char	*s;
														char	 d[4];

														s = (char *)&vf;

														d[0] = s[3];
														d[1] = s[2];
														d[2] = s[1];
														d[3] = s[0];

														MK_Write (fh, &d, sizeof (d));
													}
												#endif
											}
											else
											{
												#if __LITTLEENDIAN__
													{
														char	*s;
														char	 d[4];

														s = (char *)&vf;

														d[0] = s[3];
														d[1] = s[2];
														d[2] = s[1];
														d[3] = s[0];

														MK_Write (fh, &d, sizeof (d));
													}
												#else
													MK_Write (fh, &vf, sizeof (float));
												#endif
											}
										}
									}
									break;
								case PART_STRING:
									{
										MK_Write (fh, part->string, strlen (part->string));
									}
									break;
								case PART_BINC:
									{
										CopyIntoFile (fh, part->string);
									}
                                    break;
								case PART_ALIGN:
									{
                                        WritePadding (fh, part->size);
									}
									break;
								case PART_PAD:
									{
                                        WritePadding (fh, part->size);
									}
									break;
								}

								part = (Part*)LST_Next (part);
							}
						}
					}
					else if (fc->Size)
					{
						if (Verbose)
						{
                        	long	lc_block;
                        	long	lc_offset;
                        
                        	lc_block    = BytesWritten / ChunkSize;
                        	lc_offset   = BytesWritten % ChunkSize;
                            
                    		EL_printf ("Block $%04lx : Offset $%04lx : ", lc_block, lc_offset);
							EL_printf ("Writing %7ld bytes from %s\n", fc->Size, LST_NodeName (fc));
						}

						if (Pack)
						{
							uint8	*data;
	
							data = fc->Data;
							MK_Write (fh, data, fc->Size);
						}
						else
						{
							if (!CopyIntoFile (fh, LST_NodeName (fc)))
							{
								FailMess ("Trouble writing to %s\n", ARG_OUTFILE);
							}
						}
					}
				
                    WritePadding (fh, fc->PadSize - fc->Size);

					pos = (PositionNode*)LST_Next (pos);

					if ((LST_EndOfList (pos) && PadEnd) || (!LST_EndOfList (pos) && FilePosition(pos->fc) / ChunkSize != offset / ChunkSize))
					{
						//
						// pad to next sector
						//
						long	pad;

						if (offset + fc->PadSize != BytesWritten)
						{
							ErrMess ("NOffset $%08lx : BytesWritten $%08lx\n", offset + fc->PadSize, BytesWritten);
						}

						pad = ((offset + fc->PadSize) % ChunkSize);

						if (pad)
						{
							pad = ChunkSize - pad;
							slack += pad;

                            WritePadding (fh, pad);
						}

					}
					else if (LST_EndOfList (pos))
					{
						if (!PadEnd)
						{
							long	pad;
	
							pad = ((offset + fc->PadSize) % ChunkSize);
							if (pad)
							{
								EndPadBytes = ChunkSize - pad;
							}
						}
					}
				}

				// write out fixups
				if (UseFixupsMode == 2)
				{
					long currentFileSeekPosition;
					
					// get current position
					currentFileSeekPosition = CHK_Seek (fh, 0, EIO_SEEK_CURRENT);
					
					// seek back and fix old position
					CHK_Seek (fh, fixupFileSeekPosition, EIO_SEEK_BEGINNING);
					WritePosition (fh, currentFileSeekPosition, FALSE, "fixup table");
					
					// fix BytesWritten since we didn't actually write any new bytes
					BytesWritten -= sizeof (uint32);
					
					// get back to where we were
					CHK_Seek (fh, currentFileSeekPosition, EIO_SEEK_BEGINNING);
					
					// write out the fix ups
					WriteOutFixups (fh);
				}

				// pad to hardware sector					
				{
					long	pad;

					pad = BytesWritten % HardwareSectorSize;

					if (pad)
					{
						pad = HardwareSectorSize - pad;
						slack += pad;
						
						HardwarePadBytes += pad;

                        WritePadding (fh, pad);
					}
				}
			}

			if (blocksMarked > MaxBlocks)
			{
				ErrMess ("Too many blocks.  Given a chunksize of %d and a padsize of %d you can only have %d blocks max", ChunkSize, PadSize, MaxBlocks);
			}
			
			if (blocksMarked * ChunkSize - EndPadBytes + HardwarePadBytes + fixupAfterTableSize != BytesWritten)
			{
				ErrMess ("Marked %ld (%ld) blocks, wrote %ld bytes\n",
					blocksMarked,
					blocksMarked * ChunkSize,
					BytesWritten
				);
			}
			EL_printf ("%-30s : %5ld bytes : %3ld blocks : %5ld Slack bytes\n",
 				ARG_OUTFILE, BytesWritten, blocksMarked, slack);

			EL_printf ("--------------------------\n");	
			EL_printf ("max blocks   = %d\n", MaxBlocks);		
			EL_printf ("offset shift = %d\n", PositionOffsetShift);
			EL_printf ("offset mask  = 0x%08x\n", 0xFFFFFFFFL & (0xFFFFFFFF ^ ((0xFFFFFFFFL << PositionBlockShift) | 0x3)));
			EL_printf ("block  mask  = 0x%08x\n", 0xFFFFFFFFL << PositionBlockShift);
			EL_printf ("block shift  = %d\n", PositionBlockShift);

			CHK_Close (fh);
		}
/************************************  ************************************/
	}

done:
	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	if (ErrorCount)
	{
        EL_printf ("Errors: %d\n", ErrorCount);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


