/*************************************************************************
 *                                                                       *
 *                              RITEARGS.C                               *
 *                                                                       *
 *************************************************************************
 
                          Copyright 1997 Echidna                         
 
   DESCRIPTION
		Write Args 
 
   PROGRAMMERS
		Gregg A. Tavares
 
   FUNCTIONS
 
   TABS : 5
 
   HISTORY
		03/25/97 GAT: Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <echidna\ensure.h>

#include <stdlib.h>

#include <echidna\argparse.h>
#include <echidna\eerrors.h>
#include <echidna\eio.h>

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/





/**************************** T e m p l a t e ****************************/

#define	ARG_ALLARGS		(newargs[ 0])

char Usage[] = "Usage: riteargs ALLARGS\n";

ArgSpec Template[] = {
{STANDARD_ARG|MULTI_ARG,	"ALLARGS",	"", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	int		result = TRUE;
	char**	newargs;

	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		RETURN EXIT_FAILURE;
	}
	else
	{
		char	**args;
		
		args = MULTI_ARGLIST (ARG_ALLARGS);
		if (args)
		{
			while (*args)
			{
				EIO_PrintEscString (stdout, *args);
				args++;
			}
		}
	}
	RETURN (result ? EXIT_SUCCESS : EXIT_FAILURE);
}
ENDFUNCMAIN(main)



