/*************************************************************************
 *                                                                       *
 *                               LISTAPI.C                               *
 *                                                                       *
 *************************************************************************

                            Copyright 1996 Echidna

   DESCRIPTION
		Functions for dealing with linked lists.


   PROGRAMMERS
		Gregg A. Tavares

   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 GAT: Created

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"
#include "echidna/listapi.h"
#include "echidna/strings.h"
#include "echidna/memsafe.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/

#undef RETURN
#undef BEGINPROC
#undef BEGINFUNC
#undef ENDPROC
#undef ENDFUNC
#undef PROCEXIT

#define RETURN	return
#define BEGINPROC(a)	{
#define BEGINFUNC(a)	{
#define ENDPROC(a)		}
#define ENDFUNC(a)		}
#define PROCEXIT		return

/**************************** R O U T I N E S ****************************/


#define SIZETYPE(val)	(size_t)(val)
#define ND(node)	((LST_NODE *)(node))

#define LST_privNext(node)	((LST_NODE *)(((LST_NODE *)(node))->minNode.nd_next))
#define LST_privPrev(node)	((LST_NODE *)(((LST_NODE *)(node))->minNode.nd_prev))

/* Use the following macros when the expression will be a lvalue */

#define LST_NextL(node)		(((LST_NODE *)(node))->minNode.nd_next)
#define LST_PrevL(node)		(((LST_NODE *)(node))->minNode.nd_prev)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_InsertBefore                                         */
/*------------------------------------------------------------------------*/

void LST_privInsertBefore (LST_NODE *oldnode, LST_NODE *newnode)
BEGINPROC (LST_privInsertBefore)
{
	LST_NODE *temp;

	temp           = LST_privPrev(oldnode);
	LST_PrevL(newnode) = (struct LST_NODE *)temp;
	LST_NextL(newnode) = (struct LST_NODE *)oldnode;
	LST_NextL(temp)    = (struct LST_NODE *)newnode;
	LST_PrevL(oldnode) = (struct LST_NODE *)newnode;
} ENDPROC (LST_privInsertBefore)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_InsertAfter                                          */
/*------------------------------------------------------------------------*/

void LST_privInsertAfter (LST_NODE *oldnode, LST_NODE *newnode)
BEGINPROC (LST_privInsertAfter)
{
	LST_NODE *temp;
	temp           = LST_privNext(oldnode);
	LST_PrevL(newnode) = (struct LST_NODE *)oldnode;
	LST_NextL(newnode) = (struct LST_NODE *)temp;
	LST_PrevL(temp)    = (struct LST_NODE *)newnode;
	LST_NextL(oldnode) = (struct LST_NODE *)newnode;
} ENDPROC (LST_privInsertAfter)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_Remove                                               */
/*------------------------------------------------------------------------*/

void LST_privRemove (LST_NODE *node)
BEGINPROC (LST_privRemove)
{
	LST_NODE *prev;
	LST_NODE *next;

	prev = (LST_NODE *)LST_privPrev(node);
	next = (LST_NODE *)LST_privNext(node);

	LST_NextL(prev) = (struct LST_NODE *)next;
	LST_PrevL(next) = (struct LST_NODE *)prev;
} ENDPROC (LST_privRemove)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_AddHead                                              */
/*------------------------------------------------------------------------*/

void LST_privAddHead (LST_LIST *list, LST_NODE *node)
BEGINPROC (LST_privAddHead)
{
	LST_InsertAfter ((LST_NODE *)list, node);
} ENDPROC (LST_privAddHead)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_RemHead                                              */
/*------------------------------------------------------------------------*/

LST_NODE *LST_privRemHead (LST_LIST *list)
BEGINFUNC (LST_privRemHead)
{
	LST_NODE *node;

	node = LST_Head(list);
	if (!LST_EndOfList(node))
	{
		LST_Remove (node);
	}
	else
	{
		node = NULL;
	}
	RETURN (LST_NODE *)node;
} ENDFUNC (LST_privRemHead)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_AddTail                                              */
/*------------------------------------------------------------------------*/

void LST_privAddTail (LST_LIST *list, LST_NODE *node)
BEGINPROC (LST_privAddTail)
{
	LST_InsertBefore ((LST_NODE *)&list->minList.lst_tail, node);
} ENDPROC (LST_privAddTail)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_RemTail                                              */
/*------------------------------------------------------------------------*/

LST_NODE *LST_privRemTail (LST_LIST *list)
BEGINFUNC (LST_privRemTail)
{
	LST_NODE *node;

	node = LST_Tail(list);
	if (!LST_StartOfList(node))
	{
		LST_Remove (node);
	}
	else
	{
		node = NULL;
	}
	RETURN (LST_NODE *)node;
}
ENDFUNC (LST_privRemTail)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_InitList                                             */
/*------------------------------------------------------------------------*/

void LST_privInitList (LST_LIST *list)
BEGINERRPROC (LST_privInitList)
{
	list->minList.lst_head	= (LST_NODE *)&list->minList.lst_tail;
	list->minList.lst_tail	= NULL;
	list->minList.lst_tailPred	= (LST_NODE *)&list->minList.lst_head;
} ENDERRPROC (LST_privInitList)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_CreateList                                           */
/*------------------------------------------------------------------------*/

LST_LIST *LST_CreateList(const char *name)
BEGINFUNC (LST_CreateList)
{
	LST_LIST *list = NULL;
	
	if (MEM_fAllocMem(list,(sizeof (LST_LIST))))
	{
		memset ((void *)list, 0, sizeof (LST_LIST));
		LST_InitList (list);

		if (name)
		{
			if (!MEM_fAllocMem(list->name, strlen (name) + 1))
			{
				LST_DeleteList (list);
				list = NULL;
			}
			else
			{
				strcpy (list->name, name);
			}
		}
	}
	RETURN (list);
} ENDFUNC (LST_CreateList)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_EmptyList                                            */
/*------------------------------------------------------------------------*/

void LST_privEmptyList (LST_LIST *list)
BEGINPROC (LST_privEmptyList)
{
	LST_NODE	*node;

	if (list)
	{
		while ((node = LST_RemTail (list)) != NULL)
		{
			LST_DeleteNode (node);
		}
	}
} ENDPROC (LST_privEmptyList)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_DeleteList                                           */
/*------------------------------------------------------------------------*/

LST_LIST *LST_privDeleteList(LST_LIST *list)
BEGINFUNC (LST_privDeleteList)
{
	if (list)
	{
		LST_EmptyList (list);
		if (list->name)
		{
			MEM_FreeMem (list->name);
		}
		MEM_FreeMem (list);
	}

	RETURN (NULL);
} ENDFUNC (LST_privDeleteList)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_CreateNode                                           */
/*------------------------------------------------------------------------*/

LST_NODE *LST_CreateNode(size_t	 size, const char *name)
BEGINERRFUNC (LST_CreateNode)
{
	LST_NODE	*node = NULL;

	if (MEM_fAllocMem (node, SIZETYPE(size)))
	{
		memset ((void *)node, 0, size);

		if (name)
		{
			if (!MEM_fAllocMem(node->name, strlen (name) + 1))
			{
				LST_DeleteNode (node);
				node = NULL;
			}
			else
			{
				strcpy (node->name, name);
			}

		}
	}
	ERRRETURN (LST_NODE *)node;
}
ENDERRFUNC (LST_CreateNode)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_DeleteNode                                           */
/*------------------------------------------------------------------------*/

void LST_privDeleteNode(LST_NODE *node)
BEGINPROC (LST_privDeleteNode)
{
	if (node)
	{
		if (node->name)
		{
			MEM_FreeMem (node->name);
		}
		MEM_FreeMem(node);
	}
}
ENDPROC (LST_privDeleteNode)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_InsertPriFIFO                                        */
/*------------------------------------------------------------------------*/
void LST_InsertPriFIFO (LST_LIST *list, LST_NODE *node)
BEGINERRPROC (LST_InsertPriFIFO)
{
	LST_NODE	*nd;

	nd = LST_Head (list);
	while (!LST_EndOfList (nd))
	{
		if (LST_NodePriority (nd) < LST_NodePriority (node))
		{
			LST_InsertBefore (nd, node);
			ERRRETURN;
		}
		nd = LST_privNext (nd);
	}

	LST_AddTail (list, node);

} ENDERRPROC (LST_InsertPriFIFO)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_InsertPriLIFO                                        */
/*------------------------------------------------------------------------*/
void LST_InsertPriLIFO (LST_LIST *list, LST_NODE *node)
BEGINPROC (LST_InsertPriLIFO)
{
	LST_NODE	*nd;

	nd = LST_Head (list);
	while (!LST_EndOfList (nd))
	{
		if (LST_NodePriority (nd) <= LST_NodePriority (node))
		{
			LST_InsertBefore (nd, node);
			PROCEXIT;
		}
		nd = LST_privNext (nd);
	}

	LST_AddTail (list, node);

} ENDPROC (LST_InsertPriLIFO)

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_Find_A_Name                                          */
/*------------------------------------------------------------------------*/

#ifndef _EL_PLAT_SONY__
typedef int (*StrFunc)(const char *s1, const char *s2);
LST_NODE	*LST_Find_A_Name (
	LST_NODE	*node,
	const char	*str,
	int	 insensitive
)
BEGINFUNC (LST_Find_A_NAME)
{
	StrFunc strfunc;

	strfunc = (insensitive) ? (StrFunc)stricmp : (StrFunc)strcmp;
	while (! LST_EndOfList(node)) {
		if (! LST_StartOfList(node)) {
			if (! strfunc (LST_NodeName(node), str)) {
				RETURN node;
			}
		}
		node = LST_privNext (node);
	}

	RETURN NULL;
}
ENDFUNC (LST_Find_A_Name)
#endif

/*------------------------------------------------------------------------*/
/**# MODULE:LISTFUNC_OpOnList                                             */
/*------------------------------------------------------------------------*/

/*************************************************************************
                              LST_OpOnList
 *************************************************************************

   SYNOPSIS
		int LST_OpOnList (void *node, LST_OpOnListFunc func)

   PURPOSE
  		

   INPUT
		node :
		func :

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		12/01/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int LST_OpOnList (void *node, LST_OpOnListFunc func)
BEGINFUNC (LST_OpOnList)
{
	int	count = 0;

	while (!LST_IsEOList (node))
	{
		if (!LST_IsSOList (node))
		{
			count++;
			
			if (func)
			{
				node = func (node);
				if (!node)
				{
					RETURN (-1);
				}
			}
			else
			{
				node = LST_privNext (node);
			}
		}
		else
		{
			node = LST_privNext (node);
		}
	}
	RETURN count;

} ENDFUNC (LST_OpOnList)

/*************************************************************************
                        LST_privMoveListAfterNode
 *************************************************************************

   SYNOPSIS
		void LST_privMoveListAfterNode (LST_LIST* list, LST_NODE* node)

   PURPOSE
  		

   INPUT
		list :
		node :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		10/23/97 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void LST_privMoveListAfterNode (LST_LIST* list, LST_NODE* nodeToMoveStuffAfter)
BEGINPROC (LST_privMoveListAfterNode)
{
	LST_NODE*	node;
	
	while ((node = LST_RemTail (list)) != NULL)
	{
		LST_InsertAfter (nodeToMoveStuffAfter, node);
	}

} ENDPROC (LST_privMoveListAfterNode)

/*************************************************************************
                        LST_privMoveListBeforeNode
 *************************************************************************

   SYNOPSIS
		void LST_privMoveListBeforeNode (LST_LIST* list, LST_NODE* node)

   PURPOSE
  		

   INPUT
		list :
		node :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		10/23/97 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void LST_privMoveListBeforeNode (LST_LIST* list, LST_NODE* nodeToMoveStuffBefore)
BEGINPROC (LST_privMoveListBeforeNode)
{
	LST_NODE*	node;
	
	while ((node = LST_RemHead (list)) != NULL)
	{
		LST_InsertBefore (nodeToMoveStuffBefore, node);
	}
}
ENDPROC (LST_privMoveListBeforeNode)

/*************************************************************************
                             LST_AddSorted
 *************************************************************************

   SYNOPSIS
		void LST_AddSorted (LST_LIST* list, LST_NODE* node)

   PURPOSE
  		Add node to list sorted case-insensitive

   INPUT
		list :
		node :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/07/00 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void LST_privAddSorted (LST_LIST* list, LST_NODE* node)
BEGINPROC (LST_privAddSorted)
{
	LST_NODE* nd;
	
	nd = LST_Head (list);
	while (!LST_EndOfList (nd))
	{
		if (strcmp (LST_NodeName (node), LST_NodeName (nd)) < 0)
		{
			break;
		}
		nd = LST_Next (nd);
	}
	
	LST_InsertBefore (nd, node);
} ENDPROC (LST_privAddSorted)

/*************************************************************************
                             LST_AddSortedI
 *************************************************************************

   SYNOPSIS
		void LST_AddSortedI (LST_LIST* list, LST_NODE* node)

   PURPOSE
  		Add node to list sorted case-insensitive

   INPUT
		list :
		node :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO
		LST_AddSorted

   HISTORY
		08/07/00 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void LST_privAddSortedI (LST_LIST* list, LST_NODE* node)
BEGINPROC (LST_privAddSortedI)
{
	LST_NODE* nd;
	
	nd = LST_Head (list);
	while (!LST_EndOfList (nd))
	{
		if (stricmp (LST_NodeName (node), LST_NodeName (nd)) < 0)
		{
			break;
		}
		nd = LST_Next (nd);
	}
	
	LST_InsertBefore (nd, node);
} ENDPROC (LST_privAddSortedI)


