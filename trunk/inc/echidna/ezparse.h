/*************************************************************************
 *                                                                       *
 *                               EZPARSE.H                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

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
