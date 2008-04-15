/*************************************************************************
 *                                                                       *
 *                                HASH.H                                 *
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


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/11/96 : Created.

 *************************************************************************/

#ifndef EL_HASH_H
#define EL_HASH_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef	struct
{
	LST_NODE	 node;
	void		*data;
}
HashEntry;

typedef UINT32 (*HashFuncPtr)(HashEntry *ht);
typedef int (*HashCmpPtr)(HashEntry *t1, HashEntry *t2);

typedef struct
{
	long		 hashEntries;
	HashFuncPtr	 hashFunc;
	HashCmpPtr	 hashCmpFunc;
	LST_LIST	**hashTable;
}
HashTable;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

void HASH_InitHashTable (
		HashTable	*ht,
		HashFuncPtr	 hashFunc,
		HashCmpPtr	 hashCmpFunc,
		long		 hashEntries);
HashTable *HASH_DestroyHashTable (HashTable *ht);
HashTable *HASH_CreateHashTable (
		HashFuncPtr	 hashFunc,
		HashCmpPtr	 hashCmpFunc,
		long		 hashEntries);
void HASH_AddHashEntry (HashTable *ht, HashEntry *he);
HashEntry *HASH_RemoveHashEntry (HashEntry *he);
HashEntry *HASH_FindHashEntry (HashTable *ht, HashEntry *he);

#ifdef __cplusplus
}
#endif

#endif /* EL_HASH_H */





