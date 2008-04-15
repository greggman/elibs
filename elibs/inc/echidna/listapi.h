/*************************************************************************
 *                                                                       *
 *                               LISTAPI.H                               *
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
		Gregg A. Tavares

   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 GAT: Created

 *************************************************************************/

#ifndef EL_LISTAPI_H
#define EL_LISTAPI_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#ifdef __cplusplus
extern "C" {
#endif
/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/

typedef struct LST_MINNODE
{
	struct LST_NODE*	nd_next;
	struct LST_NODE*	nd_prev;
}
LST_MINNODE;

typedef struct LST_NODE
{
	LST_MINNODE		 minNode;
	uint8			 type;
	uint8			 pri;
	char			*name;
}
LST_NODE;

typedef struct LST_MINLIST
{
	LST_NODE*	lst_head;
	LST_NODE*	lst_tail;
	LST_NODE*	lst_tailPred;
}
LST_MINLIST;

typedef struct LST_LIST
{
	LST_MINLIST	 minList;
	uint8		 pri;
	uint8		 type;
	char		*name;
}
LST_LIST;

typedef void *(*LST_OpOnListFunc)(void *);

/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/

#define LST_EndOfList		LST_IsEOList
#define LST_StartOfList 	LST_IsSOList
#define LST_Next(node)		((LST_NODE *)(((LST_NODE *)(node))->minNode.nd_next))
#define LST_Prev(node)		((LST_NODE *)(((LST_NODE *)(node))->minNode.nd_prev))
#define LST_IsEOList(node)	(((LST_NODE *)(node))->minNode.nd_next == NULL)
#define LST_IsSOList(node)	(((LST_NODE *)(node))->minNode.nd_prev == NULL)
#define LST_Head(elist)		((LST_NODE *)(((LST_LIST *)(elist))->minList.lst_head))
#define LST_Tail(elist)		((LST_NODE *)(((LST_LIST *)(elist))->minList.lst_tailPred))
#define LST_IsEmpty(elist)	(((LST_LIST *)(elist))->minList.lst_tailPred == ((LST_NODE *)(elist)))

#define	LST_DeleteNode(nd)		LST_privDeleteNode((LST_NODE *)(nd))
#define	LST_InitList(elist)		LST_privInitList((LST_LIST *)(elist))
#define LST_EmptyList(elist)		LST_privEmptyList((LST_LIST *)(elist))
#define LST_DeleteList(elist)	LST_privDeleteList((LST_LIST *)(elist))
#define LST_InsertBefore(n1,n2)	LST_privInsertBefore((LST_NODE *)(n1), (LST_NODE *)(n2))
#define LST_InsertAfter(n1,n2)	LST_privInsertAfter((LST_NODE *)(n1), (LST_NODE *)(n2))
#define	LST_Remove(node)		LST_privRemove((LST_NODE *)(node))
#define LST_AddHead(elist,node)	LST_privAddHead((LST_LIST *)(elist), (LST_NODE *)(node))
#define	LST_RemHead(elist)		LST_privRemHead((LST_LIST *)(elist))
#define	LST_AddTail(elist,node)	LST_privAddTail((LST_LIST *)(elist), (LST_NODE *)(node))
#define LST_RemTail(elist)		LST_privRemTail((LST_LIST *)(elist))

// macros below do NOT work on LST_MINNODE or LST_MINLIST

#define	LST_SetName(node,newname)	((((LST_NODE *)(node))->name) = (newname))
#define	LST_AddName(node,name)		(LST_NodeName(node) = dupstr (name))
#define	LST_FreeName(node)			{ if (LST_NodeName(node)) freestr(LST_NodeName(node) }
#define LST_NodeName(node)			(((LST_NODE *)(node))->name)

#define LST_SetPriority(node,p)		((((LST_NODE *)(node))->pri) = (p))
#define LST_NodePriority(node)		(((LST_NODE *)(node))->pri)

#define	LST_FindName(nd,nm)			LST_Find_A_Name((LST_NODE*)(nd),(nm),0)
#define	LST_FindIName(nd,nm)		LST_Find_A_Name((LST_NODE*)(nd),(nm),1)

#define	LST_AddSorted(elist,node)	LST_privAddSorted((LST_LIST *)(elist), (LST_NODE*)(node))
#define	LST_AddSortedI(elist,node)	LST_privAddSortedI((LST_LIST *)(elist), (LST_NODE*)(node))

#define	LST_MoveListBeforeNode(lst,nd)	LST_privMoveListBeforeNode((LST_LIST*)(lst), (LST_NODE*)(nd))
#define	LST_MoveListAfterNode(lst,nd)	LST_privMoveListAfterNode((LST_LIST*)(lst), (LST_NODE*)(nd))

/************************** P R O T O T Y P E S **************************/

extern void LST_privDeleteNode (LST_NODE *node);
extern void LST_privInitList (LST_LIST *eelist);
extern void LST_privEmptyList (LST_LIST *eelist);
extern LST_LIST* LST_privDeleteList (LST_LIST *eelist);
extern void LST_privInsertBefore (LST_NODE *oldnode, LST_NODE *newnode);
extern void LST_privInsertAfter (LST_NODE *oldnode, LST_NODE *newdnode);
extern void LST_privRemove (LST_NODE *node);
extern void LST_privAddHead (LST_LIST *eelist, LST_NODE *node);
extern LST_NODE* LST_privRemHead (LST_LIST *eelist);
extern void LST_privAddTail (LST_LIST *eelist, LST_NODE *node);
extern LST_NODE* LST_privRemTail (LST_LIST *eelist);
extern int LST_OpOnList (void *node, LST_OpOnListFunc func);
extern void LST_privMoveListBeforeNode (LST_LIST* eelist, LST_NODE* node);
extern void LST_privMoveListAfterNode (LST_LIST* eelist, LST_NODE* node);


// functions below do NOT work on LST_MINNODE or LST_MINLIST

extern LST_LIST* LST_CreateList (const char *name);
extern LST_NODE* LST_CreateNode (size_t size, const char *name);

extern void LST_InsertPriFIFO (LST_LIST *eelist, LST_NODE *node);
extern void LST_InsertPriLIFO (LST_LIST *eelist, LST_NODE *node);

extern LST_NODE	*LST_Find_A_Name (LST_NODE *node, const char *str, int fInsensitive);
extern void LST_privAddSorted (LST_LIST* eelist, LST_NODE* node);
extern void LST_privAddSortedI (LST_LIST* eelist, LST_NODE* node);

#ifdef __cplusplus
}
#endif
#endif /* EL_LISTAPI_H */

