/*************************************************************************
 *                                                                       *
 *                               CHECKGLU.H                              *
 *                                                                       *
 *************************************************************************

                          Copyright Echidna 1996


   DESCRIPTION


   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		09/03/95 : Created.

 *************************************************************************/

#ifndef EL_CHECK_H
#define EL_CHECK_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/listapi.h"

#ifdef __cplusplus
extern "C" {
#endif
/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/

#define CHK_dupstr(str)    				pvCHK_dupstr(str, __FILE__, __LINE__)
#define CHK_freestr(str)    			pvCHK_freestr(str)
#define CHK_AllocateMemory2(size,id)	pvCHK_AllocateMemory2(size,id, __FILE__, __LINE__)  
#define CHK_CallocateMemory2(size,id)	pvCHK_CallocateMemory2(size,id, __FILE__, __LINE__) 
#define CHK_DeallocateMemory2(ptr,id)	pvCHK_DeallocateMemory2(ptr,id)
#define CHK_CreateNode2(size,name,id)	pvCHK_CreateNode2(size,name,id, __FILE__, __LINE__) 
#define CHK_DeleteNode2(nd,id)			pvCHK_DeleteNode2(nd,id)
#define CHK_CreateList2(name,id)		pvCHK_CreateList2(name,id, __FILE__, __LINE__) 
#define CHK_DeleteList2(list,id)		pvCHK_DeleteList2(list,id) 

#define CHK_ReadOpen(name)		CHK_Open((name), O_RDONLY | O_BINARY)
#define CHK_OpenRead(name)		CHK_Open((name), O_RDONLY | O_BINARY)
#define CHK_WriteOpen(name)		CHK_Open((name), O_WRONLY | O_BINARY | O_TRUNC | O_CREAT)
#define CHK_OpenWrite(name)		CHK_Open((name), O_WRONLY | O_BINARY | O_TRUNC | O_CREAT)
#define CHK_UpdateOpen(name)	CHK_Open((name), O_RDWR | O_BINARY | O_CREAT)
#define CHK_OpenUpdate(name)	CHK_Open((name), O_RDWR | O_BINARY | O_CREAT)
#define CHK_AppendOpen(name)	CHK_Open((name), O_RDWR | O_BINARY | O_CREAT | O_APPEND)
#define CHK_OpenAppend(name)	CHK_Open((name), O_RDWR | O_BINARY | O_CREAT | O_APPEND)

#define	CHK_DeleteNode(ptr,id)	LST_DeleteNode(ptr)

/************************** P R O T O T Y P E S **************************/

extern char *pvCHK_dupstr (const char *str, const char *file, int line);
extern void pvCHK_freestr (char *str);
extern void* pvCHK_AllocateMemory2 (long size, const char *id, const char *file, int line);
extern void* pvCHK_CallocateMemory2 (long size, const char *id, const char *file, int line);
extern void pvCHK_DeallocateMemory2 (void *ptr, const char *id);
extern void* pvCHK_CreateNode2 (size_t size, const char *name, const char *id, const char *file, int line);
extern void pvCHK_DeleteNode2 (void *ptr, const char *id);
extern LST_LIST* pvCHK_CreateList2 (const char *name, const char *id, const char *file, int line);
extern void pvCHK_DeleteList2 (LST_LIST *list, const char *id);

extern int CHK_Open (const char *name, int access);
extern long CHK_Read (int fh, void *buf, long size);
extern long CHK_Write (int fh, void *buf, long size);
extern void CHK_Close (int fh);
extern long CHK_Seek (int fh, long off, int orig);
extern long CHK_Tell (int fh);
extern long CHK_FileLength (int fh);
extern void CHK_FindFile (const char *filename, const char *guidename, char *pathbuff);

extern FILE* CHK_fopen (const char *filename, const char *mode);
extern void CHK_fclose (FILE *fp);
extern size_t CHK_fread (void *buf, size_t size, size_t n, FILE *fp);
extern size_t CHK_fwrite (void *buf, size_t size, size_t n, FILE *fp);
extern int CHK_fprintf (FILE *fp, const char *fmt, ...);
extern void CHK_fputc (int c, FILE *fp);
extern void CHK_fputs (FILE *fp, const char *s);
extern void CHK_fseek (FILE *fp, long offset, int whence);
extern LST_LIST* CHK_CreateList (const char *name, const char *id);
extern void* CHK_CreateNode (size_t size, const char *name, const char *id);

extern void* CHK_AllocateMemory (long size, const char *id);
extern void* CHK_CallocateMemory (long size, const char *id);
extern void CHK_DeallocateMemory (void *ptr, const char *id);

#ifdef __cplusplus
}
#endif
#endif /* EL_CHECK_H */


