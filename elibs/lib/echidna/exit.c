/*************************************************************************
 *                                                                       *
 *                                EXIT.C                                 *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
		Exit/Cleanup routines

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/11/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"

#include "echidna/ensure.h"
#include "echidna/listapi.h"
#include "echidna/exit.h"

#if _EL_OS_MSDOS__
	#include <dos.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

#define LOCAL static

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef struct {
	LST_NODE	 Node;
	ExitFunc	 Exit;
} ExitNode;

/****************************** G L O B A L S *****************************/

LST_LIST	 ExitListX;
LST_LIST	*ExitList = &ExitListX;

#if __EL_OS_MSDOS__
LOCAL		 fCtrlBreakState;		/* Save state of BREAK Off/On flag */
#endif

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


#if _EL_CC_WATCOMC__
/*********************************************************************
 *
 * getcbrk
 *
 * PURPOSE
 *		Get the current control-break setting.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		07/08/93 Thursday (dcc) - created.
 *
*/
int getcbrk(void)
{
	union REGS regs;

	regs.w.ax = 0x3300;
	intdos(&regs, &regs);
	return regs.h.dl;

} /* getcbrk */


/*********************************************************************
 *
 * setcbrk
 *
 * PURPOSE
 *		Set control-break setting to <cbrkvalue>.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns <cbrkvalue>
 *
 * HISTORY
 *		07/08/93 Thursday (dcc) - created.
 *
*/
int setcbrk(int cbrkvalue)
{
	union REGS regs;

	regs.w.ax = 0x3301;
	regs.h.dl = cbrkvalue;
	intdos(&regs, &regs);
	return cbrkvalue;

} /* setcbrk */
#endif /*_EL_CC_WATCOMC__*/


/*********************************************************************
 *
 * CallExitFuncs
 *
 * SYNOPSIS
 *		LOCAL void CallExitFuncs (void)
 *
 * PURPOSE
 *		Call all the exit routines.
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
LOCAL void CallExitFuncs (void)
{
	ExitNode	*en;

	while (en = (ExitNode *)LST_RemTail (ExitList)) {
		en->Exit ();
		LST_DeleteNode (en);
	}

	#if _EL_OS_MSDOS__
		setcbrk(fCtrlBreakState);	/* Restore BREAK Off/On state */
	#endif/*_EL_OS_MSDOS__*/

} /* CallExitFuncs */


/*********************************************************************
 *
 * AbortExit
 *
 * SYNOPSIS
 *		LOCAL void AbortExit (int sigtype)
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
LOCAL void AbortExit (int sigtype)
{
	char	*msg = NULL;

	#if _EL_OS_MSDOS__
		setcbrk(0);					/* Turn BREAK OFF while doing exit stuff */
	#endif

	switch (sigtype) {
	case SIGABRT:
		msg = "abnormal termination";
		break;
	case SIGFPE :
		msg = "Divide by zero/overflow";
		break;
	case SIGILL :
		msg = "Illegal Instruction";
		break;
	case SIGINT :
		msg = "Ctrl-C Break";
		break;
	case SIGSEGV:
		msg = "Illegal Storage Access";
		break;
	case SIGTERM:
		break;
	default:
		msg = "unknown signal (abort)";
		break;
	}
	if (msg)
		puts (msg);
	exit (EXIT_FAILURE);

} /* AbortExit */


/*********************************************************************
 *
 * RemoveExitFunc
 *
 * SYNOPSIS
 *		void RemoveExitFunc (ExitFunc ef)
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
void RemoveExitFunc (ExitFunc ef)
{
	ExitNode	*en;

	en = (ExitNode *)LST_Tail (ExitList);
	while (!LST_IsSOList (en)) {
		if (en->Exit == ef) {
			LST_Remove (en);
			LST_DeleteNode (en);
			return;
		}
		en = (ExitNode*)LST_Prev (en);
	}
} /* RemoveExitFunc */


/*********************************************************************
 *
 * AddExitFunc
 *
 * SYNOPSIS
 *		short AddExitFunc (ExitFunc ef)
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
short AddExitFunc (ExitFunc ef)
{
	static short init = FALSE;
	ExitNode	*en;

	if (!init) {
		LST_InitList (ExitList);
		atexit (CallExitFuncs);

		#if _EL_CC_WATCOMC__
			signal (SIGBREAK,AbortExit);
		#endif

		signal (SIGABRT, AbortExit);
		signal (SIGFPE , AbortExit);
		signal (SIGILL , AbortExit);
		signal (SIGINT , AbortExit);
		signal (SIGSEGV, AbortExit);
		signal (SIGTERM, AbortExit);

		#if _EL_OS_MSDOS__
				fCtrlBreakState = getcbrk();	/* Get BREAK Off/On state */
		#endif

		init = TRUE;
	}
	en = (ExitNode *)LST_CreateNode (sizeof (ExitNode), NULL);
	if (!en) {
		return FALSE;
	}
	en->Exit = ef;
	LST_AddTail (ExitList, en);
	return TRUE;

} /* AddExitFunc */
