/*************************************************************************
 *                                                                       *
 *                               MEMFILE.H                               *
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
