/*************************************************************************
 *                                                                       *
 *                                DBMESS.C                               *
 *                                                                       *
 *************************************************************************

                            Copyright 1996 Echidna

   DESCRIPTION
      Error, warning and failure message routines.

   PROGRAMMERS
		John M. Alvarado, Gregg Tavares

   FUNCTIONS

   TABS : 4 7

   HISTORY
		07/09/96 GAT: Created

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "echidna/ensure.h"
#include "echidna/dbmess.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/

static int NumErrors  = 0;
static int NumWarnings   = 0;

/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/


#if EL_DEBUG_MESSAGES

/*************************************************************************
                             ErrMess_Debug
 *************************************************************************

   SYNOPSIS
      void ErrMess_Debug (char *pszFileName, int LineNum)

   PURPOSE
      To print the filename and line where the error occured (and function 
      name if function names are turned on in ENSURE.H).  The main reason 
      this is a function instead of just a macro is to allow the 
      programmer to put a statement in here that will drop the program 
      into a debugger.  

   INPUT
      pszCaller   : Calling functions name.
		pszFileName : Filename string of file where error occured.
      LineNum     : Line number in file where error occured.

   OUTPUT
   	Prints error message to standard out.

   EFFECTS


   SEE ALSO


   HISTORY
		08/11/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ErrMess_Debug (char *pszCaller, char *pszFileName, int LineNum)
BEGINERRPROC (ErrMess_Debug)
{
   EL_printf(" ERROR %s(%s %d): ", pszCaller, pszFileName, LineNum);
   ++NumErrors;
} ENDERRPROC (ErrMess_Debug)


/*************************************************************************
                             WarnMess_Debug
 *************************************************************************

   SYNOPSIS
      void WarnMess_Debug (char *pszFileName, int LineNum)

   PURPOSE
      To print the filename and line where the warning occured (and function 
      name if function names are turned on in ENSURE.H).  The main reason 
      this is a function instead of just a macro is to allow the 
      programmer to put a statement in here that will drop the program 
      into a debugger.  


   INPUT
      pszCaller   : Calling functions name.
		pszFileName : Filename string of file where error occured.
      LineNum     : Line number in file where error occured.

   OUTPUT
   	Prints warning message to standard out.

   EFFECTS

   SEE ALSO
      

   HISTORY
		08/11/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void WarnMess_Debug (char *pszCaller, char *pszFileName, int LineNum)
BEGINERRPROC (WarnMess_Debug)
{
   ++NumWarnings;
   EL_printf(" WARNING %s(%s %d): ", pszCaller, pszFileName, LineNum);
} ENDERRPROC (WarnMess_Debug)

/*************************************************************************
                            ErrorSummary_Debug
 *************************************************************************

   SYNOPSIS
		int ErrorSummary_Debug (void)

   PURPOSE
      To print the number of errors that occured.

   INPUT
		None

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      Number of errors reported.

   SEE ALSO
      WarnSummary

   HISTORY
		09/10/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int ErrorSummary_Debug (void)
BEGINERRFUNC (ErrorSummary_Debug)
{
   if (NumErrors)
   {
      EL_printf ("\n%d ERROR%s!\n", NumErrors, pszIf(NumErrors>1,"S"));
   }

   ERRRETURN NumErrors;

} ENDERRFUNC (ErrorSummary_Debug)

/*************************************************************************
                          WarningSummary_Debug
 *************************************************************************

   SYNOPSIS
		int WarningSummary_Debug (void)

   PURPOSE
      To porint the number of warnings that occured.

   INPUT
		None

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      Number of warnings reported.

   SEE ALSO


   HISTORY
		09/10/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int WarningSummary_Debug (void)
BEGINERRFUNC (WarningSummary_Debug)
{

   if (NumWarnings)
   {
      EL_printf ("\n%d WARNING%s!\n", NumWarnings, pszIf(NumWarnings>1,"S"));
   }
   ERRRETURN NumWarnings;
} ENDERRFUNC (WarningSummary_Debug)

#endif

