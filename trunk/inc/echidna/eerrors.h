/*************************************************************************
 *                                                                       *
 *                               EERRORS.H                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
		Macros, Constants, Global Variables and Function to deal with
		Errors.

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 GAT: Created.

 *************************************************************************/

#ifndef EL_EERRORS_H
#define EL_EERRORS_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/

#define ERR_OUT_OF_MEMORY	1
#define ERR_FILE_NOT_FOUND	2
#define ERR_READING_FILE	3
#define ERR_REQUIREMENTS	4
#define ERR_INVALID_DATA	5
#define ERR_GENERIC			6

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS	(0)
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE	(20)
#endif

/******************************** T Y P E S *******************************/

#if _EL_CC_TURBOC__
typedef void interrupt (*SysErrorFuncPtr)(void);
#elif (_EL_OS_AMIGAOS__)
typedef void (*SysErrorFuncPtr)(void);
#elif (_EL_CC_WATCOMC__  && _EL_OS_MSDOS__)
typedef void (*SysErrorFuncPtr)(void);
#elif (_EL_OS_WIN32__)
typedef void (*SysErrorFuncPtr)(void);
#elif (_EL_OS_MACOS__)
typedef void (*SysErrorFuncPtr)(void);
#elif (_EL_OS_IRIX53__)
typedef void (*SysErrorFuncPtr)(void);
#else
#error Need something
#endif

/****************************** G L O B A L S *****************************/

extern int		 GlobalErr;
extern char		*GlobalErrMsg;
extern char		*GlobalErrStr[];
extern char		*CurrentFuncName;
extern int far32 ErrReturnCode;

extern int	ErrorCount;
extern int	WarnCount;
extern int  EL_fVerbose;

/******************************* M A C R O S ******************************/

#if EL_ERRORS_OFF

	#define SetErrMsg(msg)	strcpy (GlobalErrMsg,(msg))

	#define SetErrMsg(msg)
	
	#define GEprintf(fmt)
	#define GEprintf1(fmt,a1)
	#define GEprintf2(fmt,a1,a2)
	#define GEprintf3(fmt,a1,a2,a3)
	#define GEprintf4(fmt,a1,a2,a3,a4)
	#define GEprintf5(fmt,a1,a2,a3,a4,a5)
	#define GEprintf6(fmt,a1,a2,a3,a4,a5,a6)
	#define GEprintf7(fmt,a1,a2,a3,a4,a5,a6,a7)
	#define GEprintf8(fmt,a1,a2,a3,a4,a5,a6,a7,a8)
	#define GEprintf9(fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9)
	
	#define GEcatf(fmt)
	#define GEcatf1(fmt,a1)
	#define GEcatf2(fmt,a1,a2)
	#define GEcatf3(fmt,a1,a2,a3)
	#define GEcatf4(fmt,a1,a2,a3,a4)
	#define GEcatf5(fmt,a1,a2,a3,a4,a5)
	#define GEcatf6(fmt,a1,a2,a3,a4,a5,a6)
	#define GEcatf7(fmt,a1,a2,a3,a4,a5,a6,a7)
	#define GEcatf8(fmt,a1,a2,a3,a4,a5,a6,a7,a8)
	#define GEcatf9(fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9)

#else

	#define GEprintf(fmt)								\
		__PrintError(0,fmt)
	#define GEprintf1(fmt,a1)							\
		__PrintError(0,fmt,(a1))
	#define GEprintf2(fmt,a1,a2)						\
		__PrintError(0,fmt,(a1),(a2))
	#define GEprintf3(fmt,a1,a2,a3)						\
		__PrintError(0,fmt,(a1),(a2),(a3))
	#define GEprintf4(fmt,a1,a2,a3,a4)					\
		__PrintError(0,fmt,(a1),(a2),(a3),(a4))
	#define GEprintf5(fmt,a1,a2,a3,a4,a5)				\
		__PrintError(0,fmt,(a1),(a2),(a3),(a4),(a5))
	#define GEprintf6(fmt,a1,a2,a3,a4,a5,a6)			\
		__PrintError(0,fmt,(a1),(a2),(a3),(a4),(a5),(a6))
	#define GEprintf7(fmt,a1,a2,a3,a4,a5,a6,a7)			\
		__PrintError(0,fmt,(a1),(a2),(a3),(a4),(a5),(a6),(a7))
	#define GEprintf8(fmt,a1,a2,a3,a4,a5,a6,a7,a8)		\
		__PrintError(0,fmt,(a1),(a2),(a3),(a4),(a5),(a6),(a7),(a8))
	#define GEprintf9(fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9)	\
		__PrintError(0,fmt,(a1),(a2),(a3),(a4),(a5),(a6),(a7),(a8),(a9))
	
	#define GEcatf(fmt)									\
		__PrintError(1,(fmt))
	#define GEcatf1(fmt,a1)								\
		__PrintError(1,(fmt),(a1))
	#define GEcatf2(fmt,a1,a2)							\
		__PrintError(1,(fmt),(a1),(a2))
	#define GEcatf3(fmt,a1,a2,a3)						\
		__PrintError(1,(fmt),(a1),(a2),(a3))
	#define GEcatf4(fmt,a1,a2,a3,a4)					\
		__PrintError(1,(fmt),(a1),(a2),(a3),(a4))
	#define GEcatf5(fmt,a1,a2,a3,a4,a5)					\
		__PrintError(1,(fmt),(a1),(a2),(a3),(a4),(a5))
	#define GEcatf6(fmt,a1,a2,a3,a4,a5,a6)				\
		__PrintError(1,(fmt),(a1),(a2),(a3),(a4),(a5),(a6))
	#define GEcatf7(fmt,a1,a2,a3,a4,a5,a6,a7)			\
		__PrintError(1,(fmt),(a1),(a2),(a3),(a4),(a5),(a6),(a7))
	#define GEcatf8(fmt,a1,a2,a3,a4,a5,a6,a7,a8)		\
		__PrintError(1,(fmt),(a1),(a2),(a3),(a4),(a5),(a6),(a7),(a8))
	#define GEcatf9(fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9)		\
		__PrintError(1,(fmt),(a1),(a2),(a3),(a4),(a5),(a6),(a7),(a8),(a9))

#endif /* EL_ERRORS_OFF */

#define	SetVerboseFlag(f)	EL_fVerbose = f;

#if EL_NOSHOW_MEM
	#define PrintFreeMem()
#else
	#define PrintFreeMem()	__PrintFreeMem()
#endif

#if EL_NO_ERROR_NUMBERS
	#define SetGlobalErr(val)
	#define ClearGlobalErr()
#else
	#define SetGlobalErr(val)	GlobalErr = (val)
	#define ClearGlobalErr()	GlobalErr = 0
#endif

#if _EL_CC_LATTICE__
	#define ERRINFO(fmt) "__FILE__?" ":%d:" "__FUNC__?" ":" fmt, __LINE__
#else
	#define ERRINFO(fmt) __FILE__ ":%d:" __FUNC__ ":" fmt, __LINE__
#endif

#define GEreportf(ecode,fmt)									\
	SetGlobalErr(ecode)											\
	GEcatf((fmt))
#define GEreportf1(ecode,fmt,a1)								\
	SetGlobalErr(ecode)											\
	GEcatf1((fmt),(a1))
#define GEreportf2(ecode,fmt,a1,a2)								\
	SetGlobalErr(ecode)											\
	GEcatf2((fmt),(a1),(a2))
#define GEreportf3(ecode,fmt,a1,a2,a3)							\
	SetGlobalErr(ecode)											\
	GEcatf3((fmt),(a1),(a2),(a3))
#define GEreportf4(ecode,fmt,a1,a2,a3,a4)						\
	SetGlobalErr(ecode)											\
	GEcatf4((fmt),(a1),(a2),(a3),(a4))
#define GEreportf5(ecode,fmt,a1,a2,a3,a4,a5)					\
	SetGlobalErr(ecode)											\
	GEcatf5((fmt),(a1),(a2),(a3),(a4),(a5))
#define GEreportf6(ecode,fmt,a1,a2,a3,a4,a5,a6)					\
	SetGlobalErr(ecode)											\
	GEcatf6((fmt),(a1),(a2),(a3),(a4),(a5),(a6))
#define GEreportf7(ecode,fmt,a1,a2,a3,a4,a5,a6,a7)				\
	SetGlobalErr(ecode)											\
	GEcatf7((fmt),(a1),(a2),(a3),(a4),(a5),(a6),(a7))
#define GEreportf8(ecode,fmt,a1,a2,a3,a4,a5,a6,a7,a8)			\
	SetGlobalErr(ecode)											\
	GEcatf8((fmt),(a1),(a2),(a3),(a4),(a5),(a6),(a7),(a8))
#define GEreportf9(ecode,fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9)		\
	SetGlobalErr(ecode)											\
	GEcatf9((fmt),(a1),(a2),(a3),(a4),(a5),(a6),(a7),(a8),(a9))

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern void			ClearGlobalError (void);
extern void			__PrintError(int append, char *fmt, ...);
extern void			__PrintFreeMem(void);
extern void			SetSysErrorFunc (SysErrorFuncPtr sefptr);
extern void			FailMess	(char *fmt, ...);
extern void			ErrMess		(char *fmt, ...);
extern void			WarnMess	(char *fmt, ...);
extern void			VerboseMess	(char *fmt, ...);
extern void			OutOfMemErr	(char *fmt, ...);

#if _EL_OS_MSDOS__
#if !_EL_CC_WATCOMC__
extern void	interrupt IgnoreErrors (void);
#endif
#endif

#if _EL_OS_MACOS__
extern char *MacSystemErrMsg (int err);
#endif

#ifdef __cplusplus
}
#endif

#endif /* EL_EERRORS_H */
