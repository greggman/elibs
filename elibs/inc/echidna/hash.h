/*************************************************************************
 *                                                                       *
 *                                HASH.H                                 *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

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





