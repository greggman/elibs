/*************************************************************************
 *                                                                       *
 *                              ARGPARSE.H                               *
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
		07/09/96 GAT: Created (based on Echidna code by GAT)

 *************************************************************************/

#ifndef EL_ARGPARSE_H
#define EL_ARGPARSE_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"
#include "echidna/listapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/
#define	STANDARD_ARG	1		/* Like Filename */
#define	KEYWORD_ARG		2		/* Arg that requires a prefix keyword */
#define	SWITCH_ARG		3		/* TRUE if arg keyword is specified */
#define TOGGLE_ARG		4

#define CHRKEYWORD_ARG	(KEYWORD_ARG | CHR_ARG)
#define	CHRSWITCH_ARG	(SWITCH_ARG  | CHR_ARG)
#define	CHRTOGGLE_ARG	(TOGGLE_ARG  | CHR_ARG)

#define EXIT_ARG			(1 <<  9)	/* Argparse stops here			*/
#define	MULTI_ARG			(1 << 10)	/* Many of these args allows	*/
#define	REQUIRED_ARG		(1 << 11)	/* Arg required					*/
#define CASE_SENSITIVE_ARG	(1 << 12)	/* Arg keyword/char case snsitv	*/
#define LIST_ARG			(1 << 13)	/* (multiarg is linked list)	*/
#define EXPAND_ARG			(1 << 14)	/* Expand arg into file list	*/

/** Private: Use above constants **/
#define CHR_ARG				(1 << 15)

#define ARGTYPE(type)		(type & 0x00FF)

/***************************** T Y P E D E F S ****************************/

typedef struct {
	unsigned	 ArgType;
	char		*ArgTokens;
	char		*ArgHelp;
} ArgSpec;

/******************************* M A C R O S ******************************/

#define TOGGLE_VALUE(ptr)           (SWITCH_VALUE(ptr) == '+')
#define SWITCH_VALUE(ptr)			((int)((long)(ptr)))
#define MULTI_ARGLIST(ptr)			((char **)(ptr))
#define	MULTI_ARGLINKEDLIST(ptr)	((LST_LIST *)(ptr))

/****************** F U N C T I O N   P R O T O T Y P E S *****************/
extern int	 argify (
					char	*line,
					int	 maxargs,
					char	**args);

extern int	 arganoid (
					char	**outv,
					int		*in_argc,
					char	***in_argv,
					ArgSpec	*templat,
					int		 fail,
					int		*multi);

extern char		**argvark(
					int		*in_argc,
					char	***in_argv,
					ArgSpec	*templat,
					int		 fail);

extern char		**argparse(
					int		 in_argc,
					char	**in_argv,
					ArgSpec	*templat);

extern void		 printarghelp (
					char	*usage,
					ArgSpec	*templat);

extern void		 freeargs(
					char	**outv,
					ArgSpec	*templat);

#ifdef __cplusplus
}
#endif

#endif /* ECHIDNA_ARGPARSE_H */
