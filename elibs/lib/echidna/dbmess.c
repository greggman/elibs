/*************************************************************************
 *                                                                       *
 *                                DBMESS.C                               *
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

