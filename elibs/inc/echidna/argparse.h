/*************************************************************************
 *                                                                       *
 *                              ARGPARSE.H                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

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
