/*************************************************************************
 *                                                                       *
 *                                EXIT.H                                 *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION


   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/11/96 : Created.

 *************************************************************************/

#ifndef EL_EXIT_H
#define EL_EXIT_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef void (*ExitFunc)(void);

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern void			 RemoveExitFunc (ExitFunc ef);
extern short		 AddExitFunc (ExitFunc ef);

#ifdef __cplusplus
}
#endif

#endif /* EL_EXIT_H */
