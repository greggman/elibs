/*************************************************************************
 *                                                                       *
 *                               EERRORS.C                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
	General Error routines


   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 GAT: Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "echidna/ensure.h"
#include "echidna/strings.h"

#if _EL_CC_TURBOC__
	#include <dos.h>
	#include <alloc.h>
#endif
#if _EL_OS_AMIGAOS__
	#include <exec/memory.h>
	#if AZTEC_C
	#include <functions.h>
	#endif
	#if LATTICE
	#include <proto/exec.h>
	#endif
#endif
#if _EL_OS_MACOS__
	#include <ErrMgr.h>
#endif
#include "echidna/eerrors.h"

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS_Globals                                               */
/*------------------------------------------------------------------------*/

int		 EL_fVerbose;
int		 GlobalErr;
char	*CurrentFuncName;

char	*GlobalErrStr[] = {
	"",
	"Out of memory ",
	"Could not open file ",
	"Reading file ",
	"Failed Requirement ",
};

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS___PrintError                                          */
/*------------------------------------------------------------------------*/

static char GlobalErrMsgBuffer[1024];
char	*GlobalErrMsg = GlobalErrMsgBuffer;

/*********************************************************************
 *
 * ClearGlobalError
 *
 * SYNOPSIS
 *		void ClearGlobalError (void)
 *
 * PURPOSE
 *		Reset/Clear the global error and error msg.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void ClearGlobalError (void)
{

	GlobalErr             = 0;
	GlobalErrMsg          = GlobalErrMsgBuffer;
	GlobalErrMsgBuffer[0] = '\0';

} /* ClearGlobalError */

/**************************************************************************
 *
 * __PrintError
 *
 * SYNOPSIS
 *		void __PrintError (int append, char *fmt, ...)
 *
 * USAGE
 *		GEprintf1 ("Couldn't allocate memory");
 *
 * PURPOSE
 *		Create ErrorMessage in GlobalErrMsgBuffer.
 *
 * INPUTS
 *		fmt	: format string.
 *
 * RESULTS
 *		Prints error message and args to GlobalErrMsgBuffer and
 *		sets GlobalErrMsg to point to GlobalErrMsgBuffer.
 *
 * BUGS
 *		none.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void __PrintError (int append, char *fmt, ...)
{
	va_list		 ap;	/* points to each unnamed arg in turn */
	char		*out;

	if (append) {
		out	 = &GlobalErrMsgBuffer[strlen(GlobalErrMsgBuffer)];
	} else {
		out  = GlobalErrMsgBuffer;
		out += sprintf (out, "ERROR: ");
	}
	GlobalErrMsg = GlobalErrMsgBuffer;

	fflush (stdout);

	va_start (ap,fmt); /* make ap point to 1st unnamed arg */
	vsprintf (out,fmt,ap);
	va_end (ap);	/* clean up when done */
	
	fflush (stdout);

} /* PrintError */

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS___PrintFreeMem                                        */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * __PrintFreeMem
 *
 * PURPOSE
 *		Print amount of free memory left.
 *
 * INPUT
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
*/
void __PrintFreeMem (void)
{
#if _EL_CC_TURBOC__

	EL_printf ("freemem = %lu, %lu\n", coreleft(), farcoreleft());

#elif _EL_CC_WATCOMC__

	EL_printf ("freemem not implemented\n");

#elif _EL_OS_AMIGAOS__

	EL_printf ("freemem = %lu\n", AvailMem (NULL));

#elif _EL_OS_IRIX53__

	EL_printf ("freemem not implemented\n");

#elif _EL_OS_WIN32__

	EL_printf ("freemem not implemented\n");

#else
#error Need __PrintFreeMem() for this system.
#endif

} /* __PrintFreeMem */

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS_SetSysErrorFunc                                       */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * SetSysErrorFunc
 *
 * SYNOPSIS
 *		void SetSysErrorFunc (SysErrorFuncPtr sefptr)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
extern SysErrorFuncPtr far32 SysErrorFunc;
#if _EL_CC_TURBOC__
extern void interrupt far32 IBMErrIntHeader (void);
#endif
void SetSysErrorFunc (SysErrorFuncPtr sefptr)
{

#if _EL_CC_TURBOC__
	
	SysErrorFunc = sefptr;
	setvect (0x24, IBMErrIntHeader);

#elif _EL_CC_WATCOMC__

	sefptr = sefptr;
	EL_printf ("SetSysErrorFunc not implemented\n");

#elif _EL_OS_AMIGAOS__

	EL_printf ("SetSysErrorFunc not implemented\n");

#elif _EL_OS_MACOS__

	EL_printf ("SetSysErrorFunc not implemented\n");

#elif _EL_OS_IRIX53__

	EL_printf ("SetSysErrorFunc not implemented\n");

#elif _EL_OS_WIN32__

	EL_printf ("SetSysErrorFunc not implemented\n");

#elif
#error Need SetSysErrorFunc() for this system.
#endif

} /* SetSysErrorFunc */

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS_FailMess                                              */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * FailMess 
 *
 * SYNOPSIS
 *		void	FailMess (char *fmt, ...)
 *
 * USAGE
 *		FailMess ("Couldn't allocate memory");
 *
 * PURPOSE
 *		Print failure message and line number. Exit program.
 *
 * INPUTS
 *		fmt	: format string.
 *
 * RESULTS
 *		Prints failure message and line number to stdout. 
 *		Program terminates with exit code 20.
 *
 * BUGS
 *		none.
 *
 * HISTORY
 *	
 *
 * SEE ALSO
 *
*/
void	FailMess (char	*fmt, ...)
{
	va_list ap;	/* points to each unnamed arg in turn */

	EL_printf ("FAILED: ");

	va_start (ap,fmt); /* make ap point to 1st unnamed arg */
	EL_vprintf (fmt,ap);
	va_end (ap);	/* clean up when done */

	exit(20);

} /* FailMess */

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS_VerboseMess                                               */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * VerboseMess 
 *
 * SYNOPSIS
 *		void	VerboseMess (char *fmt, ...)
 *
 * USAGE
 *		VerboseMess ("Missing bracket");
 *
 * PURPOSE
 *		Print message if Verbose flag is TRUE
 *
 * INPUTS
 *		fmt	: format string.
 *
 * RESULTS
 *
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/

void VerboseMess (char	*fmt, ...)
{
	if (EL_fVerbose)
	{
		va_list ap;	/* points to each unnamed arg in turn */
	
		va_start (ap,fmt); /* make ap point to 1st unnamed arg */
		EL_vprintf (fmt,ap);
		va_end (ap);	/* clean up when done */
	}
} /* VerboseMess */

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS_ErrMess                                               */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * ErrMess 
 *
 * SYNOPSIS
 *		void	ErrMess (char *fmt, ...)
 *
 * USAGE
 *		ErrMess ("Missing bracket");
 *
 * PURPOSE
 *		Print error message and line number.
 *
 * INPUTS
 *		fmt	: format string.
 *
 * RESULTS
 *		Prints message and line number to stdout.
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/

int	ErrorCount	= 0;

void	ErrMess (char	*fmt, ...)
{
	va_list ap;	/* points to each unnamed arg in turn */

	++ErrorCount;
	EL_printf ("ERROR: ");

	va_start (ap,fmt); /* make ap point to 1st unnamed arg */
	EL_vprintf (fmt,ap);
	va_end (ap);	/* clean up when done */

} /* ErrMess */

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS_WarnMess                                              */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * WarnMess
 *
 * SYNOPSIS
 *		void	WarnMess (char *fmt, ...)
 *
 * USAGE
 *		WarnMess ("Suspicious ptr to ptr conversion.");
 *
 * PURPOSE
 *		Print warning message and line number. 
 *
 * INPUTS
 *		fmt	: format string.	
 *
 * RESULTS
 *		Prints warning message and line number to stdout.
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/

int	WarnCount	= 0;

void	WarnMess (char	*fmt, ...)
{
	va_list ap;	/* points to each unnamed arg in turn */

	++WarnCount;
	EL_printf ("WARNING: ");

	va_start (ap,fmt); /* make ap point to 1st unnamed arg */
	EL_vprintf (fmt,ap);
	va_end (ap);	/* clean up when done */

} /* WarnMess */

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS_OutOfMemErr                                           */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * OutOfMemErr
 *
 * SYNOPSIS
 *		void	OutOfMemErr (char *fmt, ...)
 *
 * USAGE
 *		OutOfMemErr ("Invalid tag: %s",ClassTagStr[classtag]);
 *
 * PURPOSE
 *		To print out of memory error messages. 
 *
 * INPUTS
 *
 *
 * RESULTS
 *
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void	OutOfMemErr (	char	*fmt, ...)
{
	va_list ap;	/* points to each unnamed arg in turn */

	EL_printf ("OUT OF MEMORY: ");

	va_start (ap,fmt); /* make ap point to 1st unnamed arg */
	EL_vprintf (fmt,ap);
	va_end (ap);	/* clean up when done */

	exit(20);
} /* OutOfMemErr */

/*------------------------------------------------------------------------*/
/**# MODULE:EERRORS_MacSystemErrMsg                                       */
/*------------------------------------------------------------------------*/

#if _EL_OS_MACOS__
char *MacSystemErrMsg (int err)
{
	static	char	errmsg[256];
	
	return GetSysErrText (err, errmsg);
}
#endif

