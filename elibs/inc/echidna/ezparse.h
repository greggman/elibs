/*************************************************************************
 *                                                                       *
 *                               EZPARSE.H                               *
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
		Routines to simply parse a file!?!?!

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/11/96 : Created.

 *************************************************************************/

#ifndef EL_EZPARSE_H
#define EL_EZPARSE_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/argparse.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/

#define MAX_PARSE_LINE	1024

#define EZPARSE_OKAY	1
#define	EZPARSE_ERROR	0
#define EZPARSE_FATAL	(-1)

/******************************** T Y P E S *******************************/

typedef short (*ParseFunc) (char **argv, void *userdata);

typedef struct {
	char		*Keyword;
	ArgSpec		*Template;
	ParseFunc	 ParseFunc;
} KeyWord;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern void		 EZParseKeepBlankLines (short flag);
extern short	 EZParseLineNo (void);
extern void		 EZParseError (char *fmt, ...);
extern void		 EZParseError1 (char *msg1, char *msg2);
extern short	 EZParseGetLine (FILE *fp, char *linebuff, short maxline, short strip);
extern void		 EZResetParseFilename (char *filename);
extern short	 EZParseFile (
					KeyWord *keywords,
					char *filename,
					short maxargs,
					void *userdata
				 );

#ifdef __cplusplus
}
#endif

#endif /* EL_EZPARSE_H */
