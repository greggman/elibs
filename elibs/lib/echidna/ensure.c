/*************************************************************************
 *                                                                       *
 *                               ENSURE.C                                *
 *                                                                       *
 *************************************************************************

                         Copyright 1996 Echidna

   DESCRIPTION
		Assertion functions.

   PROGRAMMERS
		John M. Alvarado, Gregg Tavares

   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 GAT: Created

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>

#include "echidna/ensure.h"

#if _EL_OS_WIN32__
	#include <assert.h>
	#include <windowsx.h>
#elif _EL_OS_PSXOS__   
	#include <kernel.h>
#endif

/*************************** C O N S T A N T S ***************************/

#if _EL_OS_PSXOS__   
	#define ADDR_SYS_TABLES	0x100		// Address of system tables
#endif

/******************************* T Y P E S *******************************/

#if _EL_OS_PSXOS__
	#define SYS_TABLE         	struct ToT	
	#define TASK_CONTROL_BLOCK	struct TCB	
	#define TASK_EXECUTE_QUEUE struct TCBH	
#endif

/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/

int fDebugTrace;		// TRUE if you want to print tracing info
static ELPrintFunc EL_specialOut;
BOOL EL_fPrintOnlySpecial;

#if EL_USE_FUNC_NAMES
	FUNC_INFO *pfiGlobalCrnt = NULL;		// Pointer to func info of current function.
   FUDGE arfudgELobal[MAX_FUDGE];
   int FudgeCount = 0;
#endif

/****************************** M A C R O S ******************************/

/**************************** R O U T I N E S ****************************/

#if  _EL_PLAT_SONY__ || _EL_PLAT_SGI__
static int _vsnprintf(char *out, unsigned int maxchars, const char *fmt, va_list ap)
BEGINERRFUNC (_vsnprintf)
{
   const char	*p;
   char		 format[64];
   char		*f;
   char		 c;
   unsigned int      maxcharsInit;
   int			 LoNg;
   int			 ShOrT;
   int			 DoUbLe;
   int			 done;
   double		 vfloat;

   unsigned int	 	 ival;
   unsigned long	 lval;
   unsigned short	 hval;

   maxcharsInit = maxchars;
   
   //va_start (ap,fmt); /* make ap point to 1st unnamed arg */
   for (p = fmt; maxchars && *p; p++) {
      if (*p != '%') {
         *out++ = *p;
         --maxchars;
      } else {
         done = FALSE;
         LoNg = FALSE;
         ShOrT = FALSE;
         DoUbLe = FALSE;
         format[0] = '%';
         f	= &format[1];
         while (!done) {
            c    = *++p;
            *f++ = c;
            if (c == '%') {
               done = TRUE;
            } else if (isalpha(c)) {
               switch (c) {
               case 'h':
                  ShOrT = TRUE;
                  break;
               case 'l':
                  LoNg = TRUE;
                  break;
               case 'L':
                  DoUbLe = TRUE;
                  break;
               default:
                  done = TRUE;
                  break;
               }
            }
         }
         *out = '\0';
         *f = '\0';
         switch (c) {
         case 'd':
         case 'i':
         case 'o':
         case 'u':
         case 'x':
         case 'X':
         case 'c':
            {
               char temp[32];  // big enough to hold largest number string hopefully.
               *temp = '\0';
               if (LoNg) {
                  lval = va_arg(ap, long);
                  sprintf (temp, format, lval);
               } else if (ShOrT) {
                  hval = va_arg(ap, short);
                  sprintf (temp, format, hval);
               } else {
                  ival = va_arg(ap, int);
                  sprintf (temp, format, ival);
               }
               for (f = temp; maxchars && *f; *out++ = *f++, --maxchars) ;
            }
            break;
         case 's':
            {
               char *sval;
               for (sval = va_arg(ap, char *); maxchars && *sval; *out++ = *sval++, --maxchars) ;
            }
            break;
         case 'f':
         case 'e':
         case 'g':
         case 'E':
         case 'G':
			{
				char temp[128];  // big enough to hold largest number string hopefully.
               *temp = '\0';
			   
			   vfloat = va_arg(ap, double);
			   sprintf (temp, format, vfloat);
			   for (f = temp; maxchars && *f; *out++ = *f++, --maxchars) ;
			}
            break;
         default:
            for (f = format; maxchars && *f; *out++ = *f++, --maxchars) ;
            break;
         }
      }
   }
   *out = '\0';
   va_end (ap);	/* clean up when done */

   ERRRETURN (int)(maxcharsInit - maxchars);          
} ENDERRFUNC (_vsnprintf)
#endif

/*************************************************************************
                              EL_vsnprintf                              
 *************************************************************************

   SYNOPSIS
		int EL_vsnprintf (char *out, unsigned int maxchars, const char *fmt, va_list ap)

   PURPOSE
  		
  
   INPUT
		out :
		int :
		s   :
		fmt :
		ap  :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		02/20/97 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int EL_vsnprintf (char *out, unsigned int maxchars, const char *fmt, va_list ap)
BEGINERRFUNC (EL_vsnprintf)
{
	ERRRETURN _vsnprintf (out, maxchars, fmt, ap);
} ENDERRFUNC (EL_vsnprintf)



#if  _EL_PLAT_SONY__
int vsprintf (char *out, const char *fmt, va_list ap)
BEGINERRFUNC (vsprintf)
{
   ERRRETURN EL_vsnprintf (out, INT_MAX, fmt, ap);
   
} ENDERRFUNC (vsprintf)
#endif


/*************************************************************************
                              EL_vprintf
 *************************************************************************

   SYNOPSIS
		int EL_printf (const char *pszFormat, va_list ap)

   PURPOSE
      Handles routine of printing depending on platform.

   INPUT
      pszFormat :  format string that was passed to va_start().
		ap        :  va_list that has already been started with va_start().

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		11/30/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int EL_vprintf (const char *pszFormat, va_list ap)
BEGINERRFUNC (EL_vprintf)
{
#define EL_VPRINTF_CHARS_MAX  1023

   char szTemp[EL_VPRINTF_CHARS_MAX+1];
   int len;

	len = EL_vsnprintf (szTemp, EL_VPRINTF_CHARS_MAX, pszFormat,ap);
	ENSURE_(len != -1 && len != EL_VPRINTF_CHARS_MAX, ("EL_printf() needs larger string buffer!\n"));
   
   #if _EL_PLAT_WIN32__

		#if 1
		{
			if (!EL_fPrintOnlySpecial)
			{
				OutputDebugString (szTemp);
				printf ("%s", szTemp);
			}
			if (EL_specialOut)
			{
				EL_specialOut (szTemp);	// supply this in your code (or use stub)
			}
		}
		#else
	  {
  		extern HWND ourMessageWindowHandle;
	    char szTemp2[EL_VPRINTF_CHARS_MAX];

		// add 0x0A to 0x0D in string (ie. add \r after \n)
		{
			char	*s,*d;

			s = szTemp;
			d = szTemp2;
			while (*s)
			{
				if (*s == '\n')
				{
					*d++ = '\r';
				}
				*d++ = *s;
				s++;
			}
			*d++ = '\0';

			ENSURE_F(d - s <= EL_VPRINTF_CHARS_MAX, ("echidna_printf() needs larger string buffer; overflow by %d\n", (d - s) - EL_VPRINTF_CHARS_MAX));
		}

		if (ourMessageWindowHandle)
		{
			{
				long	len;
	
				for (;;)
				{
					len = Edit_GetTextLength (ourMessageWindowHandle);
					if (len < 30000)
					{
						break;
					}
					Edit_SetSel (ourMessageWindowHandle, 0, len - 24000);
					SendMessage (ourMessageWindowHandle, WM_CLEAR, 0L, 0L);
					Edit_SetSel (ourMessageWindowHandle, len - (len - 24000), len - (len - 24000));
				}
			}
			Edit_ReplaceSel (ourMessageWindowHandle, szTemp2);
		}
	  }
		#endif
   #elif _EL_PLAT_SONY__
      printf(szTemp);
   #elif _EL_PLAT_SGI__
      printf(szTemp);
   #else
      #error Need code for this platform
   #endif



	ERRRETURN len;

#undef EL_VPRINTF_CHARS_MAX
} ENDERRFUNC (EL_vprintf)

#if 0
int printf (char *fmt, ...)
{
   return 0;
}
#endif

/*************************************************************************
                         EL_setSpecialPrintFunc                         
 *************************************************************************

   SYNOPSIS
		void EL_setSpecialPrintFunc (ELPrintFunc pFunc, BOOL fOnly)

   PURPOSE
  		Set up a routine to be called for all EL printing
  
   INPUT
		pFunc :
		fOnly : Only call this routine (normally calls printf too and Debugger)
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		08/08/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void EL_setSpecialPrintFunc (ELPrintFunc pFunc, BOOL fOnly)
BEGINPROC (EL_setSpecialPrintFunc)
{
	EL_specialOut = pFunc;
	EL_fPrintOnlySpecial = fOnly;

} ENDPROC (EL_setSpecialPrintFunc)


/*************************************************************************
                                 EL_printf
 *************************************************************************

   SYNOPSIS
		int EL_printf (const char *frmt, ...)

   PURPOSE
      A cross platform no hassles printf routine that we can do with what
      we want.

   INPUT
		frmt :   Format string.
      ...  :   arguments.

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		11/30/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int EL_printf (const char *frmt, ...)
BEGINERRFUNC (EL_printf)
{
	int len;

	va_list ap;	/* points to each unnamed arg in turn */

	va_start (ap,frmt); /* make ap point to 1st unnamed arg */
	len = EL_vprintf (frmt, ap);
	va_end (ap);	/* clean up when done */

	ERRRETURN len;

} ENDERRFUNC (EL_printf)

#if _EL_PLAT_SONY__
void *PSX_GetCurrentTaskId(void) 
{
	TASK_EXECUTE_QUEUE *pTaskQueue;		// Task execute queue
   void *pvId;
   
//	EnterCriticalSection();
	pTaskQueue = (TASK_EXECUTE_QUEUE *) ((SYS_TABLE *) ADDR_SYS_TABLES)[1].head;
	pvId = (void *)(pTaskQueue->entry);
//	ExitCriticalSection();
//   EL_printf("ID:0x%lX\n", (unsigned long)pvId);
	return pvId;
}
#endif

#if EL_USE_FUNC_NAMES
FUDGE *GetFudgeCurrent(void)
{
   
   FUDGE *pfudge;
   int i;
  ENS_TASKID_TYPE taskid;
  
//	EL_printf ("tid=%p\n", ENS_GetCurrentTaskID());

   taskid = ENS_GetCurrentTaskID();
   for (i = 0, pfudge=arfudgELobal; i < FudgeCount; i++, pfudge++)
   {
      if (taskid == pfudge->taskid)
//      if (ENS_GetCurrentTaskID() == pfudge->taskid)
      {
         return pfudge;  
      }
   }
#if 0   
   EL_printf("Error: GetFudgeCurrent(): Could not find owner task (0x%lX)\n", 
      (unsigned long)ENS_GetCurrentTaskID());
   for (i = 0, pfudge=arfudgELobal; i < FudgeCount; i++, pfudge++)
   {
      EL_printf("    Fudge[%d]=0x%lX:\n", i, (unsigned long)(pfudge->taskid));
   }
#endif   
   exit (EXIT_FAILURE);
   return NULL;
}
#endif

#if EL_CALLTRACE_ON
/*************************************************************************
                             TracePrintf_Debug
 *************************************************************************

   SYNOPSIS
		void TracePrintf_Debug (char *pszFormat, ...)

   PURPOSE


   INPUT
		pszFormat   :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		10/11/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void TracePrintf_Debug (char *pszFormat, ...)
{
	if (fDebugTrace)
	{
	   if (!(GetFudgeCurrent()->TraceOffCount))
	   {
   		va_list	ap;
   		va_start (ap, pszFormat);
   	   EL_vprintf (pszFormat,ap);
   		va_end (ap);
	   }
	}
}

#endif

#if EL_NO_ENSURE
#else

/*************************************************************************
                                Ensure_Debug
 *************************************************************************

   SYNOPSIS
		int  Ensure_Debug (
			int Condition,
			char *pszFileName,
			int LineNum,
			FUNC_INFO *pfi,
         char *pszMessage
		)

   PURPOSE
		To check an assertion and print debug info if it fails.

   INPUT
		Condition   : Assertion condition to check.
		pszFileName : Name of file called from.
		LineNum     : Line number in file called from.
		pfi         : Pointer to calling functions info. May be NULL.
      pszMessage  : Message to print if assertion fails.

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
		Result of assertion condition check.

   SEE ALSO
		ENSURE, ENSURE_PTR, ENSURE_RANGE

   HISTORY
		08/04/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int  Ensure_Debug (
	int Condition,
	char *pszFileName,
	int LineNum,
	FUNC_INFO *pfi,
   char *pszMessage
)
BEGINERRFUNC(Ensure_Debug)
{
   
	if (!Condition)
	{
		 EL_printf("ASSERTION FAILURE (In %s in  %s at %d): %s.\n",
			pszFuncName(pfi),
			((pszFileName) ? pszFileName : "<unknown>"),
			LineNum,
         pszSure(pszMessage)
		 );
		EL_printf("       Call chain: ");
      PrintCallChain_Debug("<--", pfi);

		 #if _EL_OS_3DO_OPERA__
			Debug();
		 #elif _EL_OS_3DO_M2__
			 exit(EXIT_FAILURE);
		 #elif _EL_OS_WIN32__
			// assert(FALSE); TODO: close graphics and then allow assert
			exit (EXIT_FAILURE);
		 #else
			 exit(EXIT_FAILURE);
		 #endif
	}

  ERRRETURN (Condition);   // Must be lowercase return to avoid recursive invocation of Assert_Debug.

} ENDERRFUNC(Ensure_Debug)

/*************************************************************************
                          PrintCallChain_Debug
 *************************************************************************

   SYNOPSIS
		void PrintCallChain_Debug (char *pszDelimiter, FUNC_INFO *pfiCaller)

   PURPOSE
      To print the chain of function calls to standard out.

   INPUT
		pszDelimiter : What to print between function names.
      pfiCaller    : FUNC_INFO pointer of caller function where call chain
                     printing should start.
   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/17/95 JMA: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define MAX_CALL_CHAIN_FUNCS	50

void PrintCallChain_Debug (char *pszDelimiter, FUNC_INFO *pfiCaller)
BEGINERRPROC (PrintCallChain_Debug)
{
   FUNC_INFO *pfi;
   int i;

   for (
         i = 0, pfi = pfiCaller;  // pfiCaller(pfiLocal());
         pfi && i < MAX_CALL_CHAIN_FUNCS;
         i++, pfi = pfiCaller(pfi)
   )
   {
      EL_printf("%s%s",((i) ? pszDelimiter : ""), pszFuncName(pfi));
      #if DEBUGGING
      if (pfi == pfiCaller(pfi))
      {
         EL_printf("ooops\n");
         break;  
      }
      #endif
   }
   EL_printf("\n");
   
   if (i >= MAX_CALL_CHAIN_FUNCS)
   {
	EL_printf ("******** Call chain deeper than %d functions\n", MAX_CALL_CHAIN_FUNCS);
   }

} ENDERRPROC (PrintCallChain_Debug)

#endif
