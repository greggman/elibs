/*************************************************************************
 *                                                                       *
 *                               ENSURE.H                                *
 *                                                                       *
 *************************************************************************

                         Copyright 1996 Echidna

   DESCRIPTION
		Macros for doing assertions and other debugging type stuff.

   PROGRAMMERS
		John M. Alvarado, Gregg Tavares

   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 GAT: Created.

 *************************************************************************/

#ifndef EL_ENSURE_H
#define EL_ENSURE_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#if (_EL_OS_IRIX53__)
   #include <sys/types.h>
   #include <unistd.h>
#endif

#if (_EL_OS_WIN32__)
	#include <process.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*************************** C O N S T A N T S ***************************/

#define MAX_FUNC_NAMES	128

#if (_EL_OS_PSXOS__)
   #ifndef EXIT_FAILURE
      #define EXIT_FAILURE    1
   #endif
   #ifndef EXIT_SUCCESS
      #define EXIT_SUCCESS    0
   #endif
   #if (EXIT_FAILURE==EXIT_SUCCESS)
      #error You better sort out this exit stuff.
   #endif
#endif

/******************************* T Y P E S *******************************/

typedef struct FUNC_INFO {
	struct FUNC_INFO *pfiCaller; // pointer to func info of calling func.
	char *pszFuncName;
} FUNC_INFO; // fi

#if (_EL_OS_IRIX53__)
   typedef pid_t ENS_TASKID_TYPE;
#elif (_EL_OS_WIN32__)
   typedef DWORD ENS_TASKID_TYPE;
#elif (_EL_OS_PSXOS__)
   typedef void *ENS_TASKID_TYPE;
#else
   #error ENS_TASKID_TYPE not defined for this system.
#endif

#define FUDGE_TRACEOFF  (1 << 0)
typedef struct {
   FUNC_INFO *pfiGlobalCrnt;
   ENS_TASKID_TYPE      taskid;
   UINT32    TraceOffCount;  
} FUDGE;

typedef void (*ELPrintFunc)(const char* s);

/***************************** G L O B A L S *****************************/

extern int fDebugTrace;

/****************************** M A C R O S ******************************/

#if (_EL_OS_IRIX53__)
   #define ENS_GetCurrentTaskID() getpid()
#elif (_EL_OS_WIN32__)
   #define ENS_GetCurrentTaskID() GetCurrentThreadId()
#elif (_EL_OS_PSXOS__)
extern void *PSX_GetCurrentTaskId(void);
   #define ENS_GetCurrentTaskID() PSX_GetCurrentTaskId()
#else
   #error ENS_GetCurrentTaskID() not defined for this system.
#endif


#if EL_USE_FUNC_NAMES
   #define MAX_FUDGE    8
   extern FUDGE arfudgELobal[MAX_FUDGE];
   extern int FudgeCount;
   extern FUDGE *GetFudgeCurrent(void);
   #define SetPfiGlobalCrnt(val)    (GetFudgeCurrent()->pfiGlobalCrnt = (val))
   #define GetPfiGlobalCrnt()       (GetFudgeCurrent()->pfiGlobalCrnt)
#else
   #define SetPfiGlobalCrnt(val)    
   #define GetPfiGlobalCrnt()       NULL
#endif

// Macro to return given string if condition is true else returns empty string.
#define pszIf(cond,psz) ((cond) ? psz : "")

// Macro to return given string if string pointer is not NULL, else returns empty string.
#define pszSure(psz)    ((psz) ? psz : "")

#if EL_USE_FUNC_NAMES
	#define pfiLocal()				(&__fi__)
	#define pfiCaller(pfi)			((pfi)->pfiCaller)
	#define pszFuncName(pfi)		((pfi)->pszFuncName)
	#define pszFuncNameLocal()		pszFuncName(pfiLocal())	
	#define pszFuncNameCaller()		pszFuncName(pfiCaller(pfiLocal()))	
#endif

#if EL_NO_ENSURE
	#define	ENSURE_F(Condition, printfargs)		          NULL
	#define ENSURE_(Condition, pszMessage)                 NULL
	#define ENSURE_PTR_(ptr, pszMessage)                   NULL
	#define ENSURE_RANGE_(var,minin,maxin, pszMessage)     NULL

	#define ENSURE(Condition)              NULL
	#define ENSURE_PTR(ptr)		            NULL
	#define ENSURE_RANGE(var,minin,maxin)  NULL
	
	#define	NOFAIL_F(Condition, printfargs)	(Condition)
	#define	NOFAIL_(Condition, pszMessage)	(Condition)
	#define	NOFAIL(Condition)				(Condition)
#else
	#define ENSURE_F(Condition, printfargs)   if (!(Condition)) {                                            \
												   EL_printf("ASSERTION FAILURE (In %s in  %s at %d): ",	    \
															pszFuncNameLocal(), __FILE__, __LINE__),   \
												   EL_printf printfargs; EL_printf ("\n");                        \
												   EL_printf("       Call chain: ");                           \
												   PrintCallChain_Debug("<--", pfiLocal());                 \
												   exit (EXIT_FAILURE);                                                \
											 }
	#define ENSURE_(Condition, pszMessage)             Ensure_Debug(Condition, __FILE__, __LINE__, pfiLocal(), pszMessage)
	#define ENSURE_PTR_(ptr,pszMessage)		            ENSURE_(ptr != NULL, pszMessage)
	#define ENSURE_RANGE_(var,minin,maxin,pszMessage)	ENSURE_(((var) >= (minin)) && ((var) <= (maxin)), pszMessage)
	
	#define ENSURE(Condition)              Ensure_Debug (Condition, __FILE__, __LINE__, pfiLocal(), \
														 "(" #Condition ") does not hold")
	#define ENSURE_PTR(ptr)		            Ensure_Debug ((ptr != NULL), __FILE__, __LINE__, pfiLocal(), \
														 #ptr " is NULL" )
	#define ENSURE_RANGE(var,minin,maxin)  Ensure_Debug (((var) >= (minin)) && ((var) <= (maxin)), \
														 __FILE__, __LINE__, pfiLocal(), \
														 #var " is out of range (" #minin "<=" #var "<=" #maxin ")")
														 
	#define NOFAIL_F	ENSURE_F
	#define	NOFAIL_		ENSURE_
	#define	NOFAIL		ENSURE
	
#endif

#if EL_USE_FUNC_NAMES
   #if EL_CALLTRACE_ON
      //#define TraceMessSdel otmnt(args)  EL_printf("      TRACE %s(%s %d): ", pszFuncNameLocal(), __FILE__, __LINE__), EL_printf args; 
      //#define TraceMessExpr(args)   EL_printf("      TRACE %s(%s %d): ", pszFuncNameLocal(), __FILE__, __LINE__), EL_printf args,
      #define TraceMessStmnt(args)  TracePrintf_Debug ("      TRACE %s(%s %d): ", pszFuncNameLocal(), __FILE__, __LINE__), TracePrintf_Debug args; 
      #define TraceMessExpr(args)   TracePrintf_Debug ("      TRACE %s(%s %d): ", pszFuncNameLocal(), __FILE__, __LINE__), TracePrintf_Debug args,
      #define TraceMessOff()  (GetFudgeCurrent()->TraceOffCount++)
      #define TraceMessOn()   (GetFudgeCurrent()->TraceOffCount--)
   #else
      #define TraceMessStmnt(args)     
      #define TraceMessExpr(args)     
      #define TraceMessOff()  
      #define TraceMessOn()   
   #endif

	#define BEGINFUNC(name)	{	FUNC_INFO __fi__;						      \
      								__fi__.pfiCaller = GetPfiGlobalCrnt();		\
      								__fi__.pszFuncName = #name;				\
      								 SetPfiGlobalCrnt(&__fi__);              \
                               TraceMessStmnt(("IN\n"))

   #define EnsureCallChain()   ENSURE_((!GetPfiGlobalCrnt() || GetPfiGlobalCrnt()==&__fi__), \
                                 "'return' used where 'RETURN' required in a function called by this routine.")
	#define ENDFUNC(name)	}
	#define BEGINPROC(name)	BEGINFUNC(name)
	#define ENDPROC(name)		goto ENDPROCLABEL; ENDPROCLABEL: EnsureCallChain(); TraceMessExpr(("OUT\n")) SetPfiGlobalCrnt(pfiCaller(pfiLocal()));}

   /*
   ** Use BEGINEXIT ENDEXIT for void functions that always execute exit().  
   ** This will avoid compiler warnings about unreachable code caused by 
   ** using ENDPROC.
   */
	#define BEGINEXIT(name)	BEGINFUNC(name)   
	#define ENDEXIT(name)	}

	#define RETURN	   return EnsureCallChain(), TraceMessExpr(("OUT\n")) (SetPfiGlobalCrnt(pfiCaller(pfiLocal()))),
	#define PROCEXIT     goto ENDPROCLABEL;

   #define PrintCallChain(pszDelimiter)   PrintCallChain_Debug(pszDelimiter, pfiLocal())

   /*
   ** BEGINERR ENDERR are used for error handling routines which need to be 
   ** exluded from manipulating the global call chain ptr because they may be 
   ** called by interrupt functions.  The global call chain ptr is made 
   ** invisible by declaring a local variable of the same name.
   */
   #define BEGINERRFUNC(name)  {  FUNC_INFO __fi__;                   \
                              __fi__.pfiCaller = &__fi__;	\
                              __fi__.pszFuncName = #name;
   #define ENDERRFUNC(name)    }
   #define BEGINERRPROC(name)  BEGINERRFUNC(name)
   #define ENDERRPROC(name)    ENDERRFUNC(name)
   #define ERRRETURN return 


   #define BEGINFUNCINT(name) BEGINFUNC(name)
   #define ENDFUNCINT(name)   ENDFUNC(name)
   #define BEGINPROCINT(name) BEGINPROC(name)
   #define ENDPROCINT(name)   ENDPROC(name)

   #define INTRETURN          RETURN

   #define BEGINFUNCMAIN(name)      { arfudgELobal[FudgeCount++].taskid = ENS_GetCurrentTaskID();                          \
                                       SetPfiGlobalCrnt(NULL);                                                       \
                                       BEGINFUNC(name)
   #define ENDFUNCMAIN(name)        ENDFUNC(name) }
   #define BEGINPROCMAIN(name)      BEGINFUNCMAIN(name)
   #define ENDPROCMAIN(name)        ENDPROC(name) }


#else					   
	#define BEGINPROC(name)    {
	#define ENDPROC(name)      }
	#define BEGINFUNC(name)    {
	#define ENDFUNC(name)      }
	#define BEGINEXIT(name)	   {
	#define ENDEXIT(name)      }
	#define BEGINERRFUNC(name) {
	#define ENDERRFUNC(name)   }
	#define BEGINERRPROC(name) {
	#define ENDERRPROC(name)   }
   #define BEGINFUNCINT(name) {
   #define ENDFUNCINT(name)   }
   #define BEGINPROCINT(name) {
   #define ENDPROCINT(name)   }
	#define pfiLocal()			NULL
	#define pfiCaller(pfi)		NULL
	#define pszFuncName(pfi)	"<unknown>"
	#define pszFuncNameLocal()	"<unknown>"
	#define pszFuncNameCaller()	"<unknown>"
	#define PrintCallChain(pszDelimiter)		NULL
	#define RETURN	return 
   #define PROCEXIT   return
   #define ERRRETURN return 
   #define INTRETURN return

   #define BEGINFUNCMAIN(name)      {
   #define ENDFUNCMAIN(name)        }
   #define BEGINPROCMAIN(name)      {
   #define ENDPROCMAIN(name)        }

   #define TraceMessStmnt(args)     
   #define TraceMessExpr(args)     
   #define TraceMessOff()  
   #define TraceMessOn()   

#endif


// Backward compatablity macros
   #define BEGINERR(name)     BEGINERRPROC(name)   
   #define ENDERR(name)       ENDERRPROC(name)


/************************** P R O T O T Y P E S **************************/

#if EL_NO_ENSURE
#else
   extern int  Ensure_Debug (
   	int Condition,
   	char *pszFileName,
   	int LineNum,
   	FUNC_INFO *pfi,
      char *pszMessage
   );
   
   extern void PrintCallChain_Debug (char *pszDelimiter, FUNC_INFO *pfiCaller);
#endif

extern void EL_setSpecialPrintFunc (ELPrintFunc pFunc, BOOL fOnly);
extern int EL_printf (const char *frmt, ...);
extern int EL_vprintf (const char *pszFormat, va_list ap);
extern int EL_vsnprintf (char *psz, unsigned int maxchars, const char *pszFormat, va_list ap);
#if _EL_PLAT_SONY__
extern int vsprintf (char *psz, const char *pszFormat, va_list ap);
#endif

#if EL_CALLTRACE_ON
   extern void TracePrintf_Debug (char *pszFormat, ...);
#endif

#ifdef __cplusplus
}
#endif

#endif /* EL_ENSURE_H */





