/*************************************************************************
 *                                                                       *
 *                               MEMFILE.H                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION


   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		03/28/96 : Created.

 *************************************************************************/

#ifndef MEMFILE_H
#define MEMFILE_H
/**************************** I N C L U D E S ****************************/

#include "stdafx.h"
#include "types.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/

typedef struct
{
	uint8	*buffer;
	uint8	*curPtr;
	long	 size;
	long	 bytesLeft;
}
MEMFILE;

/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


#define	MEMFILE_getc(mf) \
	(((mf)->bytesLeft > 0) ? (((mf)->bytesLeft--),(*mf->curPtr++)) : EOF)

/************************** P R O T O T Y P E S **************************/

extern MEMFILE *MEMFILE_Load (const CString &filename);
extern void MEMFILE_Close (MEMFILE *mf);
extern int MEMFILE_Read(MEMFILE *mf, void *buf, long len);
extern int MEMFILE_Seek (MEMFILE *mf, long pos, int type);

#endif  /* MEMFILE_H */
