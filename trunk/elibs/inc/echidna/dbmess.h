/*************************************************************************
 *                                                                       *
 *                               DBMESS.H                                *
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

#ifndef EL_DBMESS_H
#define EL_DBMESS_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <stdlib.h>
#include <stdio.h>
#include "echidna/ensure.h"

#ifdef __cplusplus
extern "C" {
#endif
/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/***************************** G L O B A L S *****************************/

/****************************** M A C R O S ******************************/

#if EL_DEBUG_MESSAGES
    #define FailureMess(args)    ErrMess_Debug(pszFuncNameLocal(), __FILE__, __LINE__), EL_printf args, EL_printf("\n       "), \
                                    PrintCallChain("<--"), EL_printf("Exiting program.\n"), exit (1)
    #define WarningMess(args)    WarnMess_Debug(pszFuncNameLocal(), __FILE__, __LINE__), EL_printf args, EL_printf("\n")
    #define InfoMess(args)       EL_printf(" INFO %s(%s %d): ", pszFuncNameLocal(), __FILE__, __LINE__), \
                                    EL_printf args 
    #define InfoMessAppend(args) EL_printf args
    #define DebugMess(args)      EL_printf args
    #define ErrorSummary()       ErrorSummary_Debug()
    #define WarningSummary()     WarningSummary_Debug()

#else
    #define FailureMess(args)    exit(1)
    #define WarningMess(args)    
    #define InfoMess(args)       
    #define InfoMessAppend(args)
    #define DebugMess(args)      

    #define ErrorSummary()       0
    #define WarningSummary()     0
#endif

/************************** P R O T O T Y P E S **************************/

#if EL_DEBUG_MESSAGES
   extern void ErrMess_Debug	(char *pszCaller, char *pszFileName, int LineNum);
   extern void WarnMess_Debug	(char *pszCaller, char *pszFileName, int LineNum);
   extern int ErrorSummary_Debug (void);
   extern int WarningSummary_Debug (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* EL_DBMESS_H */
