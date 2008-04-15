/*************************************************************************
 *                                                                       *
 *                                HASH.C                                 *
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


   PROGRAMMERS
		Routines for doing hash tables


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/11/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include <ctype.h>

#include "echidna/listapi.h"
#include "echidna/hash.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

/*********************************************************************
 *
 * hashGetLong
 *
 * SYNOPSIS
 *		char * hashGetLong (char *s, ULONG *l)
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
char *hashGetLong (char *s, UINT32 *l)
{
	UINT32	v[4];
	int		i;

	for (i = 0; i < 4; i++)
	{
		v[i] = (*s) ? tolower(*s) : 0;
		s   += (*s) ? 1  : 0;
	}

	*l = ((v[3] << 24) | (v[2] << 16) | (v[1] << 8) | (v[0]));

	return s;
}

/*********************************************************************
 *
 * defHashFunc
 *
 * SYNOPSIS
 *		static UINT32  defHashFunc (HashEntry *he)
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
static UINT32 defHashFunc (HashEntry *he)
{
	UINT32	hash = 0;
	UINT32	val  = 0;
	char	*s;

	s = LST_NodeName (he);
	while (*s)
	{
		s    = hashGetLong (s, &val);
		hash = hash ^ val;
	}

	return hash;
}

/*********************************************************************
 *
 * defHashCmpFunc
 *
 * SYNOPSIS
 *		int  defHashCmpFunc (HashEntry *t1, HashEntry *t2)
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
static int defHashCmpFunc (HashEntry *t1, HashEntry *t2)
{
	return (!stricmp (LST_NodeName (t1), LST_NodeName(t2)));
}

/*********************************************************************
 *
 * HASH_InitHashTable
 *
 * SYNOPSIS
 *		void  HASH_InitHashTable (
 *				HashTable	*ht,
 *				HashFuncPtr	 hashFunc,
 *				HashCmpPtr	 hashCmpFunc,
 *				long		 hashEntries)
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
void HASH_InitHashTable (
		HashTable	*ht,
		HashFuncPtr	 hashFunc,
		HashCmpPtr	 hashCmpFunc,
		long		 hashEntries)
{
	LST_LIST	**hashList;

	ht->hashFunc    = hashFunc    ? hashFunc    : defHashFunc;
	ht->hashCmpFunc = hashCmpFunc ? hashCmpFunc : defHashCmpFunc;
	ht->hashEntries = hashEntries;

	hashList = ht->hashTable;
	while (hashEntries)
	{
		LST_InitList (*hashList);
		hashList++;
		hashEntries--;
	}
}

/*********************************************************************
 *
 * HASH_DestroyHashTable
 *
 * SYNOPSIS
 *		HashTable * HASH_DestroyHashTable (HashTable *ht)
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
HashTable *HASH_DestroyHashTable (HashTable *ht)
{
	if (ht)
	{
		if (ht->hashTable)
		{
			long i;

			for (i = ht->hashEntries - 1; i >= 0; i--)
			{
				if (ht->hashTable[i])
				{
					LST_DeleteList (ht->hashTable[i]);
				}
			}

			free (ht->hashTable);
		}
		free (ht);
	}

	return NULL;
}

/*********************************************************************
 *
 * HASH_CreateHashTable
 *
 * SYNOPSIS
 *		HashTable * HASH_CreateHashTable (
 *				HashFuncPtr	 hashFunc,
 *				HashCmpPtr	 hashCmpFunc,
 *				long		 hashEntries)
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
HashTable *HASH_CreateHashTable (
		HashFuncPtr	 hashFunc,
		HashCmpPtr	 hashCmpFunc,
		long		 hashEntries)
{
	HashTable	*ht;

	ht = calloc (sizeof (HashTable), 1);
	if (ht)
	{
		long	i;

		ht->hashTable = calloc (sizeof (LST_LIST *), hashEntries);
		if (!ht->hashTable)
		{
			goto cleanup;
		}			

		for (i = 0; i < hashEntries; i++)
		{
			ht->hashTable[i] = LST_CreateList (NULL);
			if (!ht->hashTable[i])
			{
				goto cleanup;
			}
		}

		HASH_InitHashTable (ht, hashFunc, hashCmpFunc, hashEntries);
	}

	return ht;

cleanup:
	HASH_DestroyHashTable (ht);
	return NULL;
}

/*********************************************************************
 *
 * HASH_AddHashEntry
 *
 * SYNOPSIS
 *		void  HASH_AddHashEntry (HashTable *ht, HashEntry *he)
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
void HASH_AddHashEntry (HashTable *ht, HashEntry *he)
{
	UINT32	hash;

	hash = ht->hashFunc(he) % ht->hashEntries;
// printf ("Adding hash at #%ld\n", hash);

	LST_AddTail (ht->hashTable[hash], he);
}

/*********************************************************************
 *
 * HASH_RemoveHashEntry
 *
 * SYNOPSIS
 *		HashEntry * HASH_RemoveHashEntry (HashEntry *he)
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
HashEntry *HASH_RemoveHashEntry (HashEntry *he)
{
	LST_Remove (he);

	return he;
}

/*********************************************************************
 *
 *  HASH_FindHashEntry
 *
 * SYNOPSIS
 *		HashEntry * HASH_FindHashEntry (HashTable *ht, HashEntry *he)
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
HashEntry *HASH_FindHashEntry (HashTable *ht, HashEntry *he)
{
	UINT32		 hash;
	HashEntry	*test;

	hash = ht->hashFunc(he) % ht->hashEntries;
// printf ("Finding hash at #%ld\n", hash);

	test = (HashEntry *)LST_Head (ht->hashTable[hash]);
	while (!LST_EndOfList (test))
	{
		if (!ht->hashCmpFunc(test, he))
		{
			return test;
		}
		test = (HashEntry*)LST_Next (test);
	}
	
	return NULL;
}




