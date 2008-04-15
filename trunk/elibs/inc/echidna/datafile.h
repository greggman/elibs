/*************************************************************************
 *                                                                       *
 *                              DATAFILE.H                               *
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

#ifndef EL_DATAFILE_H
#define EL_DATAFILE_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/

#define	DATAST_NOLINEINFO	0x0001
#define DATAST_ASMLINENO	0x0002

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern short StartIntelHexSection (FILE *fp, uint16 Addr);
extern short WriteIntelHex (FILE *fp, uint8 *data, size_t size);
extern short EndIntelHexSection (FILE *fp);
extern short EndIntelHexFile (FILE *fp);

extern void	 StartDataStatements (char *header, char *format, char *delimiter, short bytesperline, uint16 flags);
extern short WriteDataStatements (FILE *fp, uint8 *data, size_t size);
extern short EndDataStatements (FILE *fp);
extern void	 StartDBHEXStatements (void);
extern void	 StartDBDECStatements (void);
extern void	 StartWordStatements (char *header, char *format, char *delimiter, short wordsperlibe, uint16 flags);
extern short WriteWordStatements (FILE *fp, uint16 *data, size_t wordcount);
extern short EndWordStatements (FILE *fp);

extern int	 OpenCPEFileWrite (char *filename);
extern short OpenCPEMemSection (int fh, uint32 address);
extern short WriteCPEMemSection (int fh, void *buffer, uint32 size);
extern short CloseCPEMemSection (void);
extern short WriteEntireCPEMemSection (int fh, uint32 address, void *buffer, uint32 size);
extern short CloseCPEFileWrite (int fh);

#ifdef __cplusplus
}
#endif

#endif /* EL_DATAFILE_H */
